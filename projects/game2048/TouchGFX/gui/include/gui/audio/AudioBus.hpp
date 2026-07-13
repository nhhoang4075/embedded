/*
 * AudioBus.hpp — Interface gửi lệnh âm thanh, dùng framing 2 byte.
 *
 * Model chỉ biết AudioBus, không biết UART. Nhờ vậy có thể inject
 * FakeAudioBus khi test host-side (ghi lại lệnh đã gửi thay vì phát
 * ra dây).
 *
 * Khung dây: [ kAudioFrameStart(0xA5) ][ AudioCmd (1 byte) ]
 * ESP32 chỉ dispatch khi thấy đúng cặp — noise 1 byte lẻ bị bỏ qua.
 */
#ifndef GAME2048_GUI_AUDIO_AUDIOBUS_HPP
#define GAME2048_GUI_AUDIO_AUDIOBUS_HPP

#include "audio_protocol.h"

class AudioBus
{
public:
    virtual ~AudioBus() = default;

    /* Gửi 1 lệnh. Được đóng khung 2 byte trước khi ra dây. */
    virtual void send(AudioCmd cmd) = 0;
};

/* Cài đặt production — bọc quanh audio_uart_send(). */
class UartAudioBus : public AudioBus
{
public:
    void send(AudioCmd cmd) override;
};

#endif /* GAME2048_GUI_AUDIO_AUDIOBUS_HPP */
