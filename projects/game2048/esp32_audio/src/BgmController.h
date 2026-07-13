/*
 * BgmController — Vòng đời nhạc nền: mở file, loop, dừng.
 *
 * CẢNH BÁO libmad: KHÔNG được delete AudioGeneratorMP3. libmad cấp
 * phát buffer nội bộ qua heap_caps_malloc với caps đặc biệt, delete
 * chuẩn sẽ crash "heap_caps_free outside heap areas".
 *
 * Cách xử lý: giữ decoder singleton suốt vòng đời, chỉ delete + tạo
 * lại AudioFileSourceSD khi restart. begin() reset internal state
 * không cần realloc.
 */
#pragma once

#include <AudioFileSourceSD.h>
#include <AudioGeneratorMP3.h>
#include <AudioOutputMixer.h>

class BgmController
{
public:
    /* Khởi tạo decoder — gọi 1 lần trong setup(). stub là output đích
     * (do AudioPipeline sở hữu). path là file BGM trên thẻ SD. */
    void begin(AudioOutputMixerStub *stub, const char *path);

    /* Bật/tắt (theo lệnh CMD_BGM_PLAY / CMD_BGM_STOP từ STM32). */
    void enable();
    void disable();
    bool isEnabled() const { return m_enabled; }

    /* Gọi mỗi vòng loop() — bơm decoder, tự restart khi EOF. */
    void service();

private:
    void startPlayback();

    AudioOutputMixerStub *m_stub    = nullptr;
    const char           *m_path    = nullptr;
    AudioGeneratorMP3    *m_decoder = nullptr;   /* singleton — xem CẢNH BÁO */
    AudioFileSourceSD    *m_file    = nullptr;
    bool                  m_enabled = false;
};
