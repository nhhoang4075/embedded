#include <gui/audio/AudioBus.hpp>

extern "C" {
#include "audio_uart.h"
#include "audio_protocol.h"
}

void UartAudioBus::send(AudioCmd cmd)
{
    /* Framing 2 byte: start marker rồi tới opcode. Buffer stack cho
     * gọn — không cần alloc động. */
    uint8_t frame[2] = { kAudioFrameStart, static_cast<uint8_t>(cmd) };
    audio_uart_send(frame, sizeof(frame));
}
