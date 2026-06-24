#ifndef SCREEN2VIEW_HPP
#define SCREEN2VIEW_HPP

#include <gui_generated/screen2_screen/Screen2ViewBase.hpp>
#include <gui/screen2_screen/Screen2Presenter.hpp>
#include <gui/common/SevenSegDigits4.hpp>
#include <touchgfx/Unicode.hpp>

class Screen2View : public Screen2ViewBase
{
public:
    Screen2View();
    virtual ~Screen2View() {}

    virtual void setupScreen();
    virtual void tearDownScreen();

    void updateBoard(const uint32_t board[4][4]);
    void updateScore(uint32_t score);

protected:

private:
    static const uint16_t SCORE_BUFFER_SIZE = 12;
    Unicode::UnicodeChar scoreBuffer[SCORE_BUFFER_SIZE];
    Unicode::UnicodeChar highScoreBuffer[SCORE_BUFFER_SIZE];

    /* 7-seg tạm hiển thị điểm hiện tại (góc trên-phải Screen2, y=8..26).
     * Bỏ khi font có glyph 0-9. */
    SevenSegDigits4 scoreSeg;
};

#endif // SCREEN2VIEW_HPP
