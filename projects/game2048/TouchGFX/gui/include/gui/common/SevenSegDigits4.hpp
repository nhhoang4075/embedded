/*
 * SevenSegDigits4.hpp - 4 chữ số 7-segment vẽ tay bằng Box widget.
 *
 * Dùng tạm vì font hiện chưa có glyph 0-9. Sau khi concaydanhram regen font
 * với WildcardCharacters="0123456789 /", chỉ cần bỏ class này và dùng
 * wildcard text như bình thường.
 *
 *   aaaa
 *  f    b      Mỗi chữ số 12x18 px, đoạn dày 2 px.
 *  f    b      Hiển thị 0..9999 (4 chữ số), padding zero trái.
 *   gggg
 *  e    c
 *  e    c
 *   dddd
 *
 * View phải tự gọi add() cho từng segment vì Screen::add() là protected.
 * Cách dùng trong View::setupScreen:
 *   seg.layout(170, 8, touchgfx::Color::getColorFromRGB(0,0,0));
 *   for (int i = 0; i < SevenSegDigits4::SEG_COUNT; ++i) add(seg.segment(i));
 *   seg.setNumber(0);
 *
 * Header-only để không phải tạo thêm subdir.mk trong CubeIDE.
 */
#ifndef SEVEN_SEG_DIGITS4_HPP
#define SEVEN_SEG_DIGITS4_HPP

#include <stdint.h>
#include <touchgfx/widgets/Box.hpp>
#include <touchgfx/Color.hpp>

class SevenSegDigits4
{
public:
    static const int DIGITS    = 4;
    static const int SEG_COUNT = DIGITS * 7;   /* tổng số Box, dùng cho vòng add() */

    /* Cấu hình vị trí + màu cho 28 Box. Chưa add vào màn — View tự làm.
     * Tổng width = DIGITS * 14 - 2 = 54 px ở vị trí (x0, y0). */
    void layout(int x0, int y0, touchgfx::colortype on_color)
    {
        for (int slot = 0; slot < DIGITS; ++slot)
        {
            layoutDigit(slot, x0 + slot * 14, y0, on_color);
        }
    }

    /* Truy cập Box thứ i (0..SEG_COUNT-1) để View::add(). */
    touchgfx::Box& segment(int i)
    {
        return segs[i];
    }

    /* Hiển thị số 0..9999. Số > 9999 sẽ kẹp về 9999. */
    void setNumber(uint16_t value)
    {
        if (value > 9999) value = 9999;
        uint16_t v = value;
        for (int slot = DIGITS - 1; slot >= 0; --slot)
        {
            uint8_t digit = v % 10;
            v /= 10;
            uint8_t mask = segMask(digit);
            touchgfx::Box *d = &segs[slot * 7];
            for (int i = 0; i < 7; ++i)
            {
                bool on = ((mask >> i) & 1u) != 0u;
                if (d[i].isVisible() != on)
                {
                    d[i].setVisible(on);
                    d[i].invalidate();
                }
            }
        }
    }

private:
    touchgfx::Box segs[SEG_COUNT];

    void layoutDigit(int slot, int x0, int y0, touchgfx::colortype on_color)
    {
        touchgfx::Box *d = &segs[slot * 7];
        /* a (top horiz)    */ d[0].setPosition(x0 + 1,  y0 + 0, 10, 2);
        /* b (top-right)    */ d[1].setPosition(x0 + 10, y0 + 1,  2, 8);
        /* c (bot-right)    */ d[2].setPosition(x0 + 10, y0 + 9,  2, 8);
        /* d (bottom horiz) */ d[3].setPosition(x0 + 1,  y0 + 16,10, 2);
        /* e (bot-left)     */ d[4].setPosition(x0 + 0,  y0 + 9,  2, 8);
        /* f (top-left)     */ d[5].setPosition(x0 + 0,  y0 + 1,  2, 8);
        /* g (middle horiz) */ d[6].setPosition(x0 + 1,  y0 + 8, 10, 2);
        for (int i = 0; i < 7; ++i)
        {
            d[i].setColor(on_color);
        }
    }

    /* Bảng đoạn cho từng chữ số: bit i = đoạn i (a..g). */
    static uint8_t segMask(uint8_t digit)
    {
        static const uint8_t MASK[10] = {
            0x3F, /* 0: a b c d e f   */
            0x06, /* 1:   b c         */
            0x5B, /* 2: a b   d e   g */
            0x4F, /* 3: a b c d     g */
            0x66, /* 4:   b c     f g */
            0x6D, /* 5: a   c d   f g */
            0x7D, /* 6: a   c d e f g */
            0x07, /* 7: a b c         */
            0x7F, /* 8: a b c d e f g */
            0x6F, /* 9: a b c d   f g */
        };
        return MASK[digit];
    }
};

#endif /* SEVEN_SEG_DIGITS4_HPP */
