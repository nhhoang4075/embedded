/* ESP32 AUDIO SLAVE cho game 2048.
 *
 * Kiến trúc: bốn module độc lập, main.cpp chỉ ghép chúng lại.
 *
 *   AudioPipeline     — sở hữu I²S out + mixer + hai stub gain.
 *   BgmController     — vòng đời nhạc nền (mở file, loop, dừng).
 *   SfxController     — WAV one-shot + audio ducking.
 *   CommandDispatcher — máy trạng thái 2 byte {0xA5, opcode}
 *                       nhận từ STM32 qua Serial2.
 *
 * Xem shared/Core/Inc/audio_protocol.h cho danh sách opcode.
 * Xem pins.h cho ánh xạ chân.
 *
 * Thẻ SD FAT32, file dưới /audio/:
 *   bgm.mp3, move.wav, merge.wav, start.wav, over.wav, highscore.wav
 */
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

#include "pins.h"
#include "AudioPipeline.h"
#include "BgmController.h"
#include "SfxController.h"
#include "CommandDispatcher.h"

static constexpr const char *BGM_PATH = "/audio/bgm.mp3";

static AudioPipeline     pipeline;
static BgmController     bgm;
static SfxController     sfx;
static CommandDispatcher dispatcher;

void setup()
{
    Serial.begin(UART_BAUD_HZ);
    delay(500);
    Serial.println("\n=== ESP32 AUDIO SLAVE ===");

    Serial2.begin(UART_BAUD_HZ, SERIAL_8N1, PIN_UART_RX, PIN_UART_TX);
    Serial.printf("UART2 RX=GPIO%d\n", PIN_UART_RX);

    /* Mount SD ở 4 MHz — thẻ Catalex chậm hơn 10 MHz thường lỗi CRC. */
    SPI.begin(PIN_SD_SCK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);
    if (!SD.begin(PIN_SD_CS, SPI, SD_SPI_CLOCK_HZ)) {
        Serial.println("[X] SD.begin FAIL");
        while (1) delay(1000);
    }
    Serial.println("[SD] mount OK");

    pipeline.begin();
    bgm.begin(pipeline.bgmStub(), BGM_PATH);
    sfx.begin(&pipeline);
    dispatcher.begin(&Serial2, &bgm, &sfx);

    /* KHÔNG auto-start BGM ở boot. Đợi STM32 gửi CMD_BGM_PLAY khi vào
     * Screen1 — tránh phát nhạc khi màn hình chưa sẵn sàng. */
}

void loop()
{
    bgm.service();
    sfx.service();
    dispatcher.poll();
}
