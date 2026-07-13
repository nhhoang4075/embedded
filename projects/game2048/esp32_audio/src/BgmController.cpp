#include "BgmController.h"
#include <Arduino.h>

void BgmController::begin(AudioOutputMixerStub *stub, const char *path)
{
    m_stub    = stub;
    m_path    = path;
    m_decoder = new AudioGeneratorMP3();
    /* KHÔNG auto-start ở boot. Đợi STM32 gửi CMD_BGM_PLAY khi vào Screen1. */
}

void BgmController::enable()
{
    m_enabled = true;
    Serial.println("[BGM] enabled");
}

void BgmController::disable()
{
    m_enabled = false;
    if (m_decoder && m_decoder->isRunning()) m_decoder->stop();
    if (m_file) { delete m_file; m_file = nullptr; }
    Serial.println("[BGM] stopped");
}

void BgmController::startPlayback()
{
    /* Xem cảnh báo libmad trong header. KHÔNG delete m_decoder. */
    if (m_decoder->isRunning()) m_decoder->stop();
    if (m_file) { delete m_file; m_file = nullptr; }
    m_file = new AudioFileSourceSD(m_path);
    if (!m_decoder->begin(m_file, m_stub)) {
        Serial.printf("[BGM] begin FAIL — file %s không đọc được\n", m_path);
        delay(2000);   /* tránh spin log; xử lý tinh hơn nếu cần */
    } else {
        Serial.printf("[BGM] play %s\n", m_path);
    }
}

void BgmController::service()
{
    if (!m_enabled) return;

    if (m_decoder->isRunning()) {
        if (!m_decoder->loop()) {
            /* EOF → tự restart để loop nhạc nền. */
            m_decoder->stop();
            startPlayback();
        }
    } else {
        startPlayback();
    }
}
