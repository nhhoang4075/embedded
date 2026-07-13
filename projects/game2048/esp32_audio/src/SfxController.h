/*
 * SfxController — Phát WAV one-shot + audio ducking với BGM.
 *
 * Khi SFX bắt đầu, gain BGM giảm để (bgm + sfx) * master không vượt
 * ngưỡng clip. Khi SFX kết thúc, gain BGM khôi phục.
 *
 * SfxController giữ tham chiếu tới AudioPipeline để điều khiển gain
 * BGM — cách gọn nhất để đóng gói ducking hoàn toàn ở đây.
 */
#pragma once

#include <AudioFileSourceSD.h>
#include <AudioGeneratorWAV.h>
#include "AudioPipeline.h"

class SfxController
{
public:
    /* Khởi tạo. pipeline dùng để điều khiển gain BGM khi ducking. */
    void begin(AudioPipeline *pipeline);

    /* Kích hoạt SFX — cắt SFX đang chạy (nếu có) rồi bắt đầu file mới.
     * Nếu file không tồn tại trên SD, log rồi bỏ qua (không crash). */
    void play(const char *path);

    /* Gọi mỗi vòng loop() — bơm decoder + quản lý gain ducking. */
    void service();

private:
    static constexpr float kBgmGainNormal = 0.7f;
    static constexpr float kBgmGainDucked = 0.1f;

    AudioPipeline      *m_pipeline = nullptr;
    AudioGeneratorWAV  *m_decoder  = nullptr;
    AudioFileSourceSD  *m_file     = nullptr;
    bool                m_ducking  = false;
};
