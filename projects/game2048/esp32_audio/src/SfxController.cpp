#include "SfxController.h"
#include <Arduino.h>
#include <SD.h>

void SfxController::begin(AudioPipeline *pipeline)
{
    m_pipeline = pipeline;
    m_decoder  = new AudioGeneratorWAV();
}

void SfxController::play(const char *path)
{
    if (!SD.exists(path)) {
        Serial.printf("[SFX miss] %s\n", path);
        return;
    }

    if (m_decoder && m_decoder->isRunning()) m_decoder->stop();
    if (m_file) { delete m_file; m_file = nullptr; }
    m_file = new AudioFileSourceSD(path);

    if (!m_decoder->begin(m_file, m_pipeline->sfxStub())) {
        Serial.printf("[SFX] begin FAIL %s\n", path);
    } else {
        Serial.printf("[SFX] play %s\n", path);
    }
}

void SfxController::service()
{
    if (m_decoder->isRunning()) {
        /* Vào ducking ở tick đầu tiên SFX chạy. */
        if (!m_ducking) {
            m_pipeline->setBgmGain(kBgmGainDucked);
            m_ducking = true;
        }
        if (!m_decoder->loop()) {
            /* SFX kết thúc — dừng + khôi phục gain BGM. */
            m_decoder->stop();
            if (m_file) { delete m_file; m_file = nullptr; }
            m_pipeline->setBgmGain(kBgmGainNormal);
            m_ducking = false;
            Serial.println("[SFX] done");
        }
    } else if (m_ducking) {
        /* SFX đã dừng nhưng chưa reset flag (edge case). */
        m_pipeline->setBgmGain(kBgmGainNormal);
        m_ducking = false;
    }
}
