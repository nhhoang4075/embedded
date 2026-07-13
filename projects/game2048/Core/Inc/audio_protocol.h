/*
 * audio_protocol.h — Giao thức âm thanh giữa STM32 và ESP32.
 *
 * File này là NGUỒN CHÂN LÝ DUY NHẤT cho mã opcode và khung truyền. Cả
 * hai firmware include CÙNG file vật lý này (STM32 qua include path
 * Core/Inc, ESP32 qua đường tương đối `../../Core/Inc/audio_protocol.h`).
 * Đừng bao giờ khai báo lại enum tương tự ở TU khác.
 *
 * Vật lý: STM32 USART1 TX PA9  →  ESP32 Serial2 RX GPIO16, 115200 8N1.
 * Đường truyền là một chiều — không có ACK, không có retransmit.
 *
 * Khung 2 byte:  [ kAudioFrameStart(0xA5) ] [ AudioCmd ]
 *   - Byte lẻ (không đi kèm start byte) bị ESP32 bỏ qua → chống noise
 *     làm phát sfx ngẫu nhiên.
 *   - Frame start 0xA5 chọn vì có mẫu bit 1010_0101 dễ đồng bộ lại.
 *
 * Thêm opcode mới: chỉ cần thêm một dòng vào enum AudioCmd, đảm bảo
 * giá trị byte trùng với ánh xạ file bên ESP32 (SfxController hoặc
 * bảng lookup).
 */
#ifndef GAME2048_SHARED_AUDIO_PROTOCOL_H
#define GAME2048_SHARED_AUDIO_PROTOCOL_H

#include <stdint.h>

#ifdef __cplusplus

/* Trong C++ dùng enum class để có type-safety. Byte trên dây là underlying
 * type uint8_t — chuyển đổi hai chiều bằng static_cast tại lớp UART. */
enum class AudioCmd : uint8_t {
    Move     = 0x01,  /* trượt ô, không gộp */
    Merge    = 0x02,  /* trượt có gộp, điểm tăng */
    Start    = 0x03,  /* ván mới bắt đầu */
    Over     = 0x04,  /* game over */
    NewHigh  = 0x05,  /* phá kỷ lục lần đầu trong ván */
    BgmPlay  = 0x06,  /* bật nhạc nền (khi vào menu) */
    BgmStop  = 0x07,  /* tắt nhạc nền (khi vào ván chơi) */
};

/* Byte mở đầu khung. Đọc thêm phần comment ở đầu file để hiểu vì sao. */
static constexpr uint8_t kAudioFrameStart = 0xA5;

#else  /* Bên ESP32 Arduino có thể include theo C thuần nếu cần. */

typedef enum {
    AUDIO_CMD_MOVE      = 0x01,
    AUDIO_CMD_MERGE     = 0x02,
    AUDIO_CMD_START     = 0x03,
    AUDIO_CMD_OVER      = 0x04,
    AUDIO_CMD_NEW_HIGH  = 0x05,
    AUDIO_CMD_BGM_PLAY  = 0x06,
    AUDIO_CMD_BGM_STOP  = 0x07,
} audio_cmd_t;

#define AUDIO_FRAME_START  ((uint8_t)0xA5)

#endif /* __cplusplus */

#endif /* GAME2048_SHARED_AUDIO_PROTOCOL_H */
