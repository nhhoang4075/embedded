#include "AudioPipeline.h"
#include "pins.h"

/* Hằng gain gộp một chỗ — muốn tinh chỉnh chỉ sửa ở đây. */
static constexpr float MASTER_GAIN     = 1.5f;
static constexpr float BGM_GAIN_NORMAL = 0.85f;   /* to hơn 0.7 một chút; (0.85+0.45)*1.5=1.95 vẫn dưới ngưỡng clip */
static constexpr float SFX_GAIN        = 0.45f;
static constexpr int   MIXER_BUFFER    = 32;

void AudioPipeline::begin()
{
    m_out = new AudioOutputI2S();
    m_out->SetPinout(PIN_I2S_BCLK, PIN_I2S_LRC, PIN_I2S_DOUT);
    m_out->SetGain(MASTER_GAIN);

    m_mixer   = new AudioOutputMixer(MIXER_BUFFER, m_out);
    m_bgmStub = m_mixer->NewInput();
    m_sfxStub = m_mixer->NewInput();

    m_bgmStub->SetGain(BGM_GAIN_NORMAL);
    m_sfxStub->SetGain(SFX_GAIN);
}

void AudioPipeline::setBgmGain(float g)
{
    if (m_bgmStub) m_bgmStub->SetGain(g);
}

void AudioPipeline::setSfxGain(float g)
{
    if (m_sfxStub) m_sfxStub->SetGain(g);
}
