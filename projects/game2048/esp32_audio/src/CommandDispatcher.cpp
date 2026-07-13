#include "CommandDispatcher.h"
#include "BgmController.h"
#include "SfxController.h"
#include "../../Core/Inc/audio_protocol.h"
#include <Arduino.h>

namespace {

/* Bảng ánh xạ opcode SFX → file. Thêm SFX mới chỉ cần một dòng. BGM
 * PLAY / STOP là hai case stateful, xử lý riêng bên dưới. */
struct SfxEntry {
    AudioCmd    cmd;
    const char *path;
};

constexpr SfxEntry kSfxTable[] = {
    { AudioCmd::Move,    "/audio/move.wav"      },
    { AudioCmd::Merge,   "/audio/merge.wav"     },
    { AudioCmd::Start,   "/audio/start.wav"     },
    { AudioCmd::Over,    "/audio/over.wav"      },
    { AudioCmd::NewHigh, "/audio/highscore.wav" },
};

} // namespace

void CommandDispatcher::begin(Stream *rx, BgmController *bgm, SfxController *sfx)
{
    m_rx    = rx;
    m_bgm   = bgm;
    m_sfx   = sfx;
    m_state = RxState::WaitStart;
}

void CommandDispatcher::poll()
{
    if (!m_rx) return;

    while (m_rx->available()) {
        uint8_t b = static_cast<uint8_t>(m_rx->read());

        if (m_state == RxState::WaitStart) {
            /* Chỉ chuyển state khi thấy đúng frame start. Byte lẻ noise
             * ở đây sẽ bị nuốt mà không phát tiếng. */
            if (b == kAudioFrameStart) {
                m_state = RxState::WaitCmd;
            } else {
                Serial.printf("[UART] drop stray 0x%02X (no frame start)\n", b);
            }
            continue;
        }

        /* State = WaitCmd → byte này là opcode. */
        dispatch(b);
        m_state = RxState::WaitStart;
    }
}

void CommandDispatcher::dispatch(uint8_t opcode)
{
    Serial.printf("[CMD] 0x%02X\n", opcode);

    /* Tìm trong bảng SFX trước. */
    for (const auto& e : kSfxTable) {
        if (static_cast<uint8_t>(e.cmd) == opcode) {
            m_sfx->play(e.path);
            return;
        }
    }

    /* BGM là hai case stateful, xử lý riêng. */
    switch (static_cast<AudioCmd>(opcode)) {
        case AudioCmd::BgmPlay:
            m_bgm->enable();
            break;
        case AudioCmd::BgmStop:
            m_bgm->disable();
            break;
        default:
            Serial.printf("[!] Unknown opcode 0x%02X\n", opcode);
            break;
    }
}
