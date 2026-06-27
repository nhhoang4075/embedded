/* ESP32 AUDIO SLAVE cho game 2048.
 *
 * Kien truc:
 *   BGM (background music) loop lien tuc tu /audio/bgm.wav.
 *   SFX trigger qua UART command tu STM32, MIX vao bgm (khong cat bgm).
 *
 * UART command map (1 byte tu STM32 PA2 -> ESP32 D16 = Serial2 RX):
 *   0x01  MOVE       /audio/move.wav    (mỗi swipe hop le)
 *   0x02  MERGE      /audio/merge.wav   (block gop)
 *   0x03  START      /audio/start.wav   (vao van moi)
 *   0x04  OVER       /audio/over.wav    (game over)
 *
 * Files trong / audio/ tren SD = WAV format. Neu file thieu, sketch
 * van chay bgm + log "[SFX miss] /path" qua USB serial - khong crash.
 */
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

#include <AudioFileSourceSD.h>
#include <AudioGeneratorWAV.h>
#include <AudioGeneratorMP3.h>
#include <AudioOutputI2S.h>
#include <AudioOutputMixer.h>

//============================
// SD Card (SPI)
//============================
#define SD_MISO    5
#define SD_MOSI    18
#define SD_SCK     19
#define SD_CS      21

//============================
// MAX98357A (I2S)
//============================
#define I2S_BCLK   26
#define I2S_LRC    25
#define I2S_DOUT   27

//============================
// UART command tu STM32
//============================
#define UART_BAUD     115200
#define UART_RX_PIN   16        // GPIO16 = D16 = Serial2 RX
#define UART_TX_PIN   17        // GPIO17 = D17 = Serial2 TX (chua dung)

enum : uint8_t {
    CMD_MOVE      = 0x01,
    CMD_MERGE     = 0x02,
    CMD_START     = 0x03,
    CMD_OVER      = 0x04,
    CMD_NEW_HIGH  = 0x05,   // break high score lan dau trong van
    CMD_BGM_PLAY  = 0x06,   // vao Screen1 -> bat bgm
    CMD_BGM_STOP  = 0x07,   // vao Screen2 -> dung bgm
};

//============================
// Audio paths
// The SD da format lai FAT32 -> dung BGM chinh thuc tu /audio/bgm.mp3.
static const char *BGM_PATH        = "/audio/bgm.mp3";
static const char *MOVE_PATH       = "/audio/move.wav";
static const char *MERGE_PATH      = "/audio/merge.wav";
static const char *START_PATH      = "/audio/start.wav";
static const char *OVER_PATH       = "/audio/over.wav";
static const char *NEW_HIGH_PATH   = "/audio/highscore.wav";

//============================
// Audio globals
//============================
static AudioOutputI2S      *out      = nullptr;
static AudioOutputMixer    *mixer    = nullptr;
static AudioOutputMixerStub *bgmStub = nullptr;
static AudioOutputMixerStub *sfxStub = nullptr;

static AudioGeneratorMP3   *bgm      = nullptr;   // BGM = mp3 (file lon, nen)
static AudioFileSourceSD   *bgmFile  = nullptr;
static bool                 bgmEnabled = false;   // chi play khi STM32 yeu cau

static AudioGeneratorWAV   *sfx      = nullptr;
static AudioFileSourceSD   *sfxFile  = nullptr;

//============================
// Helpers
//============================
static void startBgm()
{
    // KHONG delete bgm: libmad cap phat noi bo qua heap_caps_malloc voi caps
    // dac biet, delete chuan crash "heap_caps_free outside heap areas".
    // Stop decoder roi cap nhat lai source -> begin() reset internal state.
    if (bgm->isRunning()) bgm->stop();
    if (bgmFile) { delete bgmFile; bgmFile = nullptr; }
    bgmFile = new AudioFileSourceSD(BGM_PATH);
    if (!bgm->begin(bgmFile, bgmStub)) {
        Serial.printf("[BGM] begin FAIL — file %s khong doc duoc\n", BGM_PATH);
        delay(2000);
    } else {
        Serial.printf("[BGM] play %s\n", BGM_PATH);
    }
}

// Trigger SFX overlay. Cat SFX dang chay (neu co) roi bat dau cai moi.
static void playSfx(const char *path)
{
    if (!SD.exists(path)) {
        Serial.printf("[SFX miss] %s\n", path);
        return;
    }
    if (sfx && sfx->isRunning()) sfx->stop();
    if (sfxFile) { delete sfxFile; sfxFile = nullptr; }
    sfxFile = new AudioFileSourceSD(path);
    if (!sfx->begin(sfxFile, sfxStub)) {
        Serial.printf("[SFX] begin FAIL %s\n", path);
    } else {
        Serial.printf("[SFX] play %s\n", path);
    }
}

//============================
// Setup
//============================
void setup()
{
    Serial.begin(UART_BAUD);
    delay(500);
    Serial.println("\n=== ESP32 AUDIO SLAVE ===");

    // UART2 nhan command tu STM32
    Serial2.begin(UART_BAUD, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
    Serial.printf("UART2 RX=GPIO%d, TX=GPIO%d\n", UART_RX_PIN, UART_TX_PIN);

    // SD mount @ 10MHz
    SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    if (!SD.begin(SD_CS, SPI, 4000000)) {
        Serial.println("[X] SD.begin FAIL");
        while (1) delay(1000);
    }
    Serial.println("[SD] mount OK");

    // I2S output -> MAX98357A
    out = new AudioOutputI2S();
    out->SetPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    // Headroom math: (bgm_g + sfx_g) * out_g phai <= ~2.0 voi audio thuc (peak ~0.5).
    // Vuot qua thi I2S 16-bit clip -> tieng "de" khi SFX de len BGM.
    out->SetGain(1.5f);

    mixer   = new AudioOutputMixer(32, out);
    bgmStub = mixer->NewInput();
    sfxStub = mixer->NewInput();
    bgmStub->SetGain(0.7f);   // bgm nen background
    sfxStub->SetGain(0.45f);  // sfx phai du nho de khong clip khi cong voi bgm ducked

    bgm = new AudioGeneratorMP3();
    sfx = new AudioGeneratorWAV();

    // KHONG auto-start BGM o boot. Doi STM32 gui CMD_BGM_PLAY khi
    // user vao Screen1. Tranh phat nhac luc man hinh chua san sang.
}

//============================
// Loop
//============================
void loop()
{
    // ---- BGM: chi chay khi enabled (Screen1 menu). Stop khi vao Screen2. ----
    if (bgmEnabled) {
        if (bgm->isRunning()) {
            if (!bgm->loop()) {
                // EOF -> restart de loop background music.
                bgm->stop();
                startBgm();
            }
        } else {
            startBgm();
        }
    }

    // ---- SFX: chay 1 lan, hết thì dừng ----
    // Audio ducking: BGM thap khi SFX phat (tranh clip (0.5+0.7)*1.5 = 1.8).
    // SFX xong -> BGM gain restore. Pattern chuan cua game audio.
    static bool ducking = false;
    if (sfx->isRunning()) {
        if (!ducking) {
            bgmStub->SetGain(0.1f);   // BGM lui sau xuong nen khi SFX phat
            ducking = true;
        }
        if (!sfx->loop()) {
            sfx->stop();
            if (sfxFile) { delete sfxFile; sfxFile = nullptr; }
            bgmStub->SetGain(0.7f);   // restore BGM
            ducking = false;
            Serial.println("[SFX] done");
        }
    } else if (ducking) {
        bgmStub->SetGain(0.7f);
        ducking = false;
    }

    // ---- Check UART command ----
    if (Serial2.available()) {
        uint8_t cmd = Serial2.read();
        Serial.printf("[UART RX] 0x%02X\n", cmd);

        switch (cmd) {
            case CMD_MOVE:     playSfx(MOVE_PATH);     break;
            case CMD_MERGE:    playSfx(MERGE_PATH);    break;
            case CMD_START:    playSfx(START_PATH);    break;
            case CMD_OVER:     playSfx(OVER_PATH);     break;
            case CMD_NEW_HIGH: playSfx(NEW_HIGH_PATH); break;
            case CMD_BGM_PLAY:
                bgmEnabled = true;
                Serial.println("[BGM] enabled");
                break;
            case CMD_BGM_STOP:
                bgmEnabled = false;
                if (bgm->isRunning()) bgm->stop();
                if (bgmFile) { delete bgmFile; bgmFile = nullptr; }
                Serial.println("[BGM] stopped");
                break;
            default:
                Serial.printf("[!] Unknown cmd 0x%02X\n", cmd);
                break;
        }
    }
}
