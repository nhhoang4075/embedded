/*
 * flash_high_score_store.cpp — Đọc/ghi điểm cao ra flash STM32F429.
 *
 * File .cpp (không .c) vì FlashHighScoreStore là class C++ kế thừa từ
 * HighScoreStore. Cả HAL_FLASH_* đều được C++ gọi bình thường.
 */
#include "flash_high_score_store.h"
#include "board_config.h"

extern "C" {
#include "stm32f4xx_hal.h"
}

namespace {

constexpr uint32_t kMagic = 0xC0DE2048U;
constexpr uint32_t kSalt  = 0xA5A5A5A5U;

struct Record {
    uint32_t magic;
    uint32_t score;
    uint32_t checksum;
};

inline uint32_t compute_checksum(uint32_t score)
{
    return kMagic ^ score ^ kSalt;
}

inline volatile const Record* record_addr()
{
    return reinterpret_cast<volatile const Record*>(BOARD_HISCORE_FLASH_ADDR);
}

} // namespace

uint32_t FlashHighScoreStore::load()
{
    volatile const Record* r = record_addr();

    /* Flash rỗng đọc ra toàn 0xFF → magic không khớp → trả 0 (chưa lưu). */
    if (r->magic != kMagic) return 0;
    if (r->checksum != compute_checksum(r->score)) return 0;
    return r->score;
}

void FlashHighScoreStore::save(uint32_t score)
{
    HAL_FLASH_Unlock();

    /* Xoá sector chứa record. Sector 11 là 128 KB — nếu để dùng chung
     * với dữ liệu khác thì phải chuyển sang scheme "wear-leveled" phức
     * tạp hơn. Hiện tại sector này chỉ dành cho high score. */
    FLASH_EraseInitTypeDef eraseCfg = {};
    eraseCfg.TypeErase    = FLASH_TYPEERASE_SECTORS;
    eraseCfg.VoltageRange = FLASH_VOLTAGE_RANGE_3; /* 2.7V..3.6V, hỗ trợ word program */
    eraseCfg.Sector       = BOARD_HISCORE_FLASH_SECTOR;
    eraseCfg.NbSectors    = 1;
    eraseCfg.Banks        = FLASH_BANK_1;

    uint32_t sectorError = 0;
    if (HAL_FLASHEx_Erase(&eraseCfg, &sectorError) != HAL_OK) {
        HAL_FLASH_Lock();
        return; /* im lặng — save thất bại, giá trị mới chỉ còn ở RAM */
    }

    const uint32_t addr = BOARD_HISCORE_FLASH_ADDR;
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr + 0, kMagic);
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr + 4, score);
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr + 8, compute_checksum(score));

    HAL_FLASH_Lock();
}
