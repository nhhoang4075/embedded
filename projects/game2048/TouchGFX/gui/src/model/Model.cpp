#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include <cstdlib>

extern "C"
{
#include "game2048.h"
#include "joystick.h"
#include "platform_clock.h"
}

/* Adapter cho g2048_init: game core cần một callback rng theo signature
 * uint32_t(void*), ta chỉ cần đọc rand() (đã seed ở nơi khác). */
static uint32_t simulator_rng(void*)
{
    return rand();
}

Model::Model() :
    modelListener(0),
    phase(Phase::Menu),
    highScore(0),
    recordBroken(false),
    overEmitted(false),
    rngSeeded(false)
{
    /* Seed tạm thời bằng tick — sẽ được thay bằng reseed thực khi user
     * tương tác lần đầu (reseedRngIfNeeded). */
    srand(platform_now_ms());
    g2048_init(&game, simulator_rng, nullptr);

    /* Nạp điểm cao đã lưu (flash sector 11). Nếu chưa từng lưu hoặc
     * flash trắng thì store.load() trả 0. */
    highScore = scoreStore.load();
}

void Model::enterMenu()
{
    phase = Phase::Menu;
    /* BGM chỉ chạy khi ở menu — Screen1 và Screen2 tránh chồng âm. */
    playSfx(AudioCmd::BgmPlay);
}

void Model::enterGame()
{
    phase = Phase::Game;
    playSfx(AudioCmd::BgmStop);

    /* Nếu ván trước đã thua, tự động khởi động ván mới khi vào Screen2. */
    if (game.state == G2048_LOST)
    {
        resetGame();
    }
}

void Model::playSfx(AudioCmd cmd)
{
    /* Fire-and-forget qua AudioBus. Nếu ESP32 chết, tối đa mất 5 ms rồi
     * trả về (timeout trong audio_uart_send). GUI không bị treo lâu. */
    audioBus.send(cmd);
}

void Model::reseedRngIfNeeded()
{
    if (rngSeeded) return;
    /* Trộn ba nguồn entropy: tick ms + chu kỳ CPU (DWT) + noise ADC.
     * Timing của lần bấm đầu tiên không thể đoán được → seed đủ tốt
     * cho game (không cần cryptographic RNG). */
    uint32_t entropy = platform_now_ms();
    entropy ^= platform_cycles();
    entropy ^= (static_cast<uint32_t>(joystick_raw_x()) << 16) |
               static_cast<uint32_t>(joystick_raw_y());
    srand(entropy);
    rngSeeded = true;
}

void Model::resetGame()
{
    g2048_init(&game, simulator_rng, nullptr);
    recordBroken = false;
    overEmitted  = false;
    playSfx(AudioCmd::Start);
}

void Model::tick()
{
    /* SW joystick luôn được poll, kể cả trên Screen1, để điều hướng. */
    if (joystick_sw_pressed() && modelListener)
    {
        reseedRngIfNeeded();     /* lần đầu user chạm → nạp entropy thật */
        modelListener->swPressed();
    }

    /* Phát OVER sfx khi vừa transition vào LOST. Bắt được cả 2 đường:
     *   - thua tự nhiên (state đổi trong g2048_move dưới)
     *   - thua chủ động qua swPressed (presenter set state = LOST) */
    if (game.state == G2048_LOST && !overEmitted)
    {
        overEmitted = true;
        playSfx(AudioCmd::Over);

        /* Điểm cao chỉ ghi flash tại đây — endpoint sạch, không ghi
         * mỗi frame. Nếu score ván này lớn hơn giá trị cache thì lưu. */
        if (game.score > scoreStore.load())
        {
            scoreStore.save(game.score);
        }
    }

    /* Joystick analog chỉ áp dụng khi đang ở phase GAME (Screen2). */
    if (phase != Phase::Game)
        return;

    /* Đã thua thì không xử lý nước đi nữa, đợi resetGame. */
    if (game.state == G2048_LOST)
        return;

    joy_dir_t dir = joystick_poll();
    if (dir == JOY_NONE)
        return;

    uint32_t prevScore = game.score;

    if (!g2048_move(&game, (g2048_dir_t)dir))
        return;

    uint32_t newScore = game.score;
    bool     hadMerge = (newScore > prevScore);

    /* MOVE = swipe không gộp. MERGE = swipe có gộp (điểm tăng). */
    playSfx(hadMerge ? AudioCmd::Merge : AudioCmd::Move);

    /* Break high score: chỉ lần đầu vượt qua highScore trong ván này. */
    if (!recordBroken && newScore > highScore)
    {
        recordBroken = true;
        playSfx(AudioCmd::NewHigh);
    }

    if (newScore > highScore)
        highScore = newScore;

    if (modelListener)
    {
        modelListener->boardChanged();
        modelListener->scoreChanged(newScore);
    }
}
