#include <Arduino.h>
#include <Audio.h>
#include <SPI.h>
#include <SD.h>

//============================
// SD Card
//============================
#define SD_CS      15
#define SD_SCK     18
#define SD_MISO    19
#define SD_MOSI    23

//============================
// MAX98357A (I2S)
//============================
#define I2S_BCLK   26
#define I2S_LRC    25
#define I2S_DOUT   22

// UART command
#define CMD_GAME_START 0x02
#define CMD_GAME_OVER  0x03

Audio audio;
bool playingEffect = false;

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("1");

    SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    Serial.println("2");

    if (!SD.begin(SD_CS, SPI, 400000))
    {
        Serial.println("SD FAIL");
        Serial.print("Card type = ");
        Serial.println(SD.cardType());
        while(1);
    }

    Serial.println("3");

    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    Serial.println("4");

    audio.setVolume(18);

    Serial.println("5");

    bool ok = audio.connecttoFS(SD, "/mp3/august.mp3");

    Serial.println(ok ? "PLAY OK" : "PLAY FAIL");
}

void loop()
{
    audio.loop();

    //=========================
    // Nhận UART từ STM32
    //=========================
    if (Serial.available())
    {
        uint8_t cmd = Serial.read();

        switch (cmd)
        {
            case CMD_GAME_START:

                Serial.println("GAME START");

                playingEffect = true;

                audio.stopSong();

                audio.connecttoFS(SD,
                                  "/wav/game_start.wav");

                break;

            case CMD_GAME_OVER:

                Serial.println("GAME OVER");

                playingEffect = true;

                audio.stopSong();

                audio.connecttoFS(SD,
                                  "/wav/game_over.wav");

                break;
        }
    }

    //=========================
    // Hiệu ứng phát xong
    //=========================
    if (playingEffect && !audio.isRunning())
    {
        playingEffect = false;

        audio.connecttoFS(SD,
                          "/mp3/august.mp3");
    }

    //=========================
    // Nếu MP3 hết thì phát lại
    //=========================
    if (!playingEffect && !audio.isRunning())
    {
        audio.connecttoFS(SD,
                          "/mp3/august.mp3");
    }
}

//====================================================
// Callback (chỉ để debug)
//====================================================

void audio_info(const char *info)
{
    Serial.print("[INFO] ");
    Serial.println(info);
}

void audio_id3data(const char *info)
{
    Serial.print("[ID3] ");
    Serial.println(info);
}

void audio_eof_mp3(const char *info)
{
    Serial.println("End of MP3");
}

void audio_showstation(const char *info)
{
    Serial.println(info);
}

void audio_showstreaminfo(const char *info)
{
    Serial.println(info);
}

void audio_bitrate(const char *info)
{
    Serial.print("Bitrate: ");
    Serial.println(info);
}

void audio_commercial(const char *info)
{
    Serial.println(info);
}

void audio_icyurl(const char *info)
{
    Serial.println(info);
}

void audio_lasthost(const char *info)
{
    Serial.println(info);
}

void audio_eof_speech(const char *info)
{
    Serial.println(info);
}