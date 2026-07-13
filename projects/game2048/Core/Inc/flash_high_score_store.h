/*
 * flash_high_score_store.h — Impl lưu điểm cao ra flash STM32F429.
 *
 * Dùng sector 11 (128 KB cuối bank 1, địa chỉ 0x080E0000 lấy từ
 * board_config.h). Firmware không chạm tới vùng này.
 *
 * Endurance ~10 000 erase/sector. Vì chỉ ghi khi phá kỷ lục nên
 * dư dùng cả đời board.
 */
#ifndef GAME2048_FLASH_HIGH_SCORE_STORE_H
#define GAME2048_FLASH_HIGH_SCORE_STORE_H

#include "high_score_store.h"

class FlashHighScoreStore : public HighScoreStore
{
public:
    uint32_t load() override;
    void     save(uint32_t score) override;
};

#endif /* GAME2048_FLASH_HIGH_SCORE_STORE_H */
