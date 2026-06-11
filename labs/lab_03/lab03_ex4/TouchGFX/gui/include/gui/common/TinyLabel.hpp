#ifndef TINYLABEL_HPP
#define TINYLABEL_HPP

#include <touchgfx/containers/Container.hpp>
#include <touchgfx/widgets/Box.hpp>

/* Tiny text label rendered with a 3x5 block font built from Box widgets
   (no TouchGFX Designer font/text resources needed).
   Supported glyphs: A C E M O R S X ':' and space. Max 8 characters. */
class TinyLabel : public touchgfx::Container
{
public:
    static const int MAX_CHARS = 8;

    TinyLabel()
    {
        for (int i = 0; i < MAX_CHARS; i++)
        {
            for (int p = 0; p < 15; p++)
            {
                px[i][p].setVisible(false);
                add(px[i][p]);
            }
        }
    }

    /* scale = size of one font pixel in screen pixels */
    void setText(const char* s, int16_t scale)
    {
        int len = 0;
        while (s[len] != '\0' && len < MAX_CHARS)
        {
            len++;
        }

        setWidth(len * 4 * scale - scale);
        setHeight(5 * scale);

        for (int i = 0; i < MAX_CHARS; i++)
        {
            uint16_t g = (i < len) ? glyph(s[i]) : 0;
            for (int row = 0; row < 5; row++)
            {
                for (int col = 0; col < 3; col++)
                {
                    touchgfx::Box& b = px[i][row * 3 + col];
                    b.setPosition(i * 4 * scale + col * scale, row * scale, scale, scale);
                    b.setVisible(((g >> (row * 3 + col)) & 1) != 0);
                }
            }
        }
    }

    void setColor(touchgfx::colortype c)
    {
        for (int i = 0; i < MAX_CHARS; i++)
        {
            for (int p = 0; p < 15; p++)
            {
                px[i][p].setColor(c);
            }
        }
    }

private:
    /* 3x5 glyph bitmap; bit index = row*3+col, col 0 = left column */
    static uint16_t glyph(char c)
    {
        switch (c)
        {
        case 'S': return rows(0b111, 0b100, 0b111, 0b001, 0b111);
        case 'C': return rows(0b111, 0b100, 0b100, 0b100, 0b111);
        case 'O': return rows(0b111, 0b101, 0b101, 0b101, 0b111);
        case 'R': return rows(0b110, 0b101, 0b110, 0b101, 0b101);
        case 'E': return rows(0b111, 0b100, 0b111, 0b100, 0b111);
        case 'M': return rows(0b101, 0b111, 0b101, 0b101, 0b101);
        case 'A': return rows(0b010, 0b101, 0b111, 0b101, 0b101);
        case 'X': return rows(0b101, 0b101, 0b010, 0b101, 0b101);
        case ':': return rows(0b000, 0b010, 0b000, 0b010, 0b000);
        default:  return 0; // space / unsupported
        }
    }

    /* Pack 5 row literals (0b111 style, MSB = left) into one 15-bit glyph */
    static uint16_t rows(uint8_t r0, uint8_t r1, uint8_t r2, uint8_t r3, uint8_t r4)
    {
        return (uint16_t)(rev(r0) | (rev(r1) << 3) | (rev(r2) << 6) | (rev(r3) << 9) | (rev(r4) << 12));
    }

    /* Remap row literal (bit2 = left) to storage order (bit0 = left) */
    static uint16_t rev(uint8_t r)
    {
        return (uint16_t)(((r & 0b100) >> 2) | (r & 0b010) | ((r & 0b001) << 2));
    }

    touchgfx::Box px[MAX_CHARS][15];
};

#endif // TINYLABEL_HPP
