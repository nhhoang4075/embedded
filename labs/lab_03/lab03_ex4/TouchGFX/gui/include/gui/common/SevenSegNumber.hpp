#ifndef SEVENSEGNUMBER_HPP
#define SEVENSEGNUMBER_HPP

#include <touchgfx/containers/Container.hpp>
#include <touchgfx/widgets/Box.hpp>

/* Displays a number 0..999 in 7-segment LED style, drawn entirely
   with Box widgets (no TouchGFX Designer font/text resources needed) */
class SevenSegNumber : public touchgfx::Container
{
public:
    static const int NUM_DIGITS = 3;

    SevenSegNumber()
    {
        setWidth(NUM_DIGITS * DIGIT_W + (NUM_DIGITS - 1) * GAP);
        setHeight(DIGIT_H);

        for (int d = 0; d < NUM_DIGITS; d++)
        {
            int16_t ox = d * (DIGIT_W + GAP);
            /* 7 segments: A(top) B(top-right) C(bottom-right) D(bottom)
                        E(bottom-left) F(top-left) G(middle) */
            segs[d][0].setPosition(ox + T,           0,               DIGIT_W - 2 * T, T);
            segs[d][1].setPosition(ox + DIGIT_W - T, T,               T, DIGIT_H / 2 - T);
            segs[d][2].setPosition(ox + DIGIT_W - T, DIGIT_H / 2,     T, DIGIT_H / 2 - T);
            segs[d][3].setPosition(ox + T,           DIGIT_H - T,     DIGIT_W - 2 * T, T);
            segs[d][4].setPosition(ox,               DIGIT_H / 2,     T, DIGIT_H / 2 - T);
            segs[d][5].setPosition(ox,               T,               T, DIGIT_H / 2 - T);
            segs[d][6].setPosition(ox + T,           DIGIT_H / 2 - T / 2, DIGIT_W - 2 * T, T);
            for (int s = 0; s < 7; s++)
            {
                add(segs[d][s]);
            }
        }
        setValue(0);
    }

    void setColor(touchgfx::colortype c)
    {
        for (int d = 0; d < NUM_DIGITS; d++)
        {
            for (int s = 0; s < 7; s++)
            {
                segs[d][s].setColor(c);
            }
        }
    }

    void setValue(uint16_t v)
    {
        if (v > 999)
        {
            v = 999;
        }
        const uint8_t digits[NUM_DIGITS] = {
            (uint8_t)(v / 100), (uint8_t)((v / 10) % 10), (uint8_t)(v % 10)
        };
        bool blank = true;  // hide leading zeros (e.g. 7 instead of 007)
        for (int d = 0; d < NUM_DIGITS; d++)
        {
            if (digits[d] != 0 || d == NUM_DIGITS - 1)
            {
                blank = false;
            }
            uint8_t mask = blank ? 0 : segMask(digits[d]);
            for (int s = 0; s < 7; s++)
            {
                segs[d][s].setVisible(((mask >> s) & 1) != 0);
            }
        }
    }

private:
    static const int16_t DIGIT_W = 14;
    static const int16_t DIGIT_H = 24;
    static const int16_t T = 3;   // segment thickness
    static const int16_t GAP = 4; // gap between digits

    static uint8_t segMask(uint8_t digit)
    {
        switch (digit)
        {
        case 0: return 0x3F;
        case 1: return 0x06;
        case 2: return 0x5B;
        case 3: return 0x4F;
        case 4: return 0x66;
        case 5: return 0x6D;
        case 6: return 0x7D;
        case 7: return 0x07;
        case 8: return 0x7F;
        default: return 0x6F; // 9
        }
    }

    touchgfx::Box segs[NUM_DIGITS][7];
};

#endif // SEVENSEGNUMBER_HPP
