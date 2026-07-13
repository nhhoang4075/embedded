/*
 * high_score_store.h — Nơi lưu điểm cao nhất qua reset/tắt nguồn.
 *
 * Interface abstract — cho phép thay backend (RAM cho test host, Flash
 * cho production). Model chỉ biết interface, không biết cách lưu.
 *
 * Layout record 12 byte:
 *   uint32_t magic       (0xC0DE2048 — nhận diện bản ghi hợp lệ)
 *   uint32_t score       (giá trị điểm)
 *   uint32_t checksum    (magic ^ score ^ 0xA5A5A5A5)
 *
 * Đọc: verify magic khớp và checksum khớp → trả score, ngược lại trả 0.
 * Ghi: erase toàn bộ sector rồi program 3 word.
 */
#ifndef GAME2048_HIGH_SCORE_STORE_H
#define GAME2048_HIGH_SCORE_STORE_H

#include <stdint.h>

class HighScoreStore
{
public:
    virtual ~HighScoreStore() = default;

    /* Đọc giá trị lưu trữ. Nếu lỗi CRC hoặc chưa có bản ghi nào, trả 0. */
    virtual uint32_t load() = 0;

    /* Lưu giá trị mới. Impl chịu trách nhiệm erase/program. Chỉ gọi khi
     * score thực sự lớn hơn giá trị đang lưu — mỗi lần ghi tốn 1 chu kỳ
     * erase sector, không nên gọi mỗi frame. */
    virtual void save(uint32_t score) = 0;
};

/* Impl RAM-only — dùng cho unit test hoặc build simulator. */
class InMemoryHighScoreStore : public HighScoreStore
{
public:
    uint32_t load() override { return value; }
    void save(uint32_t score) override { value = score; }
private:
    uint32_t value = 0;
};

#endif /* GAME2048_HIGH_SCORE_STORE_H */
