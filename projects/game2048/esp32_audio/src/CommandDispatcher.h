/*
 * CommandDispatcher — Nhận byte UART, dispatch sang BGM/SFX.
 *
 * Máy trạng thái 2 byte theo khung {kAudioFrameStart, opcode} định
 * nghĩa trong shared audio_protocol.h:
 *
 *   State = WaitStart:
 *     đọc byte b. Nếu b == kAudioFrameStart → chuyển WaitCmd.
 *     Byte lẻ trên đường (noise) không match → bỏ.
 *
 *   State = WaitCmd:
 *     đọc byte b. Dispatch theo opcode rồi về WaitStart.
 *
 * Nhờ vậy dây UART lỡ nhiễu 1 byte sẽ không phát SFX ngẫu nhiên.
 *
 * Bảng {opcode → SFX path} đọc từ static const kSfxTable — thêm SFX
 * chỉ cần một dòng.
 */
#pragma once

#include <Stream.h>
#include <stdint.h>

class BgmController;
class SfxController;

class CommandDispatcher
{
public:
    /* Cắm các controller mà dispatch sẽ gọi. */
    void begin(Stream *rx, BgmController *bgm, SfxController *sfx);

    /* Gọi mỗi vòng loop() — xử lý mọi byte có sẵn trên rx. */
    void poll();

private:
    enum class RxState { WaitStart, WaitCmd };

    void dispatch(uint8_t opcode);

    Stream        *m_rx     = nullptr;
    BgmController *m_bgm    = nullptr;
    SfxController *m_sfx    = nullptr;
    RxState        m_state  = RxState::WaitStart;
};
