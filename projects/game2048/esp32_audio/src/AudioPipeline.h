/*
 * AudioPipeline — Sở hữu I²S out, mixer, hai stub gain.
 *
 * Không biết gì về BGM/SFX cụ thể — chỉ cung cấp:
 *   - hai AudioOutputMixerStub cho BGM và SFX (BgmController /
 *     SfxController lấy để gắn generator vào).
 *   - method chỉnh gain BGM/SFX runtime (SfxController dùng cho ducking).
 *
 * Gain math (mixer 16-bit):
 *   (bgm + sfx) * master  ≤  ~2.0    với audio peak ~0.5
 * Vượt qua thì I²S clip → tiếng "dè" khi SFX chồng BGM.
 * Ducking là cách giữ gain trong ngưỡng: khi SFX chạy, BGM giảm.
 */
#pragma once

#include <AudioOutputI2S.h>
#include <AudioOutputMixer.h>

class AudioPipeline
{
public:
    /* Bật I²S ra chân đã cấu hình + tạo mixer 2 input. Gọi 1 lần trong
     * setup(). */
    void begin();

    /* Stub gắn generator BGM/SFX vào. Ownership vẫn thuộc mixer. */
    AudioOutputMixerStub *bgmStub() { return m_bgmStub; }
    AudioOutputMixerStub *sfxStub() { return m_sfxStub; }

    /* Chỉnh gain runtime. Được SfxController gọi để ducking. */
    void setBgmGain(float g);
    void setSfxGain(float g);

private:
    AudioOutputI2S      *m_out     = nullptr;
    AudioOutputMixer    *m_mixer   = nullptr;
    AudioOutputMixerStub *m_bgmStub = nullptr;
    AudioOutputMixerStub *m_sfxStub = nullptr;
};
