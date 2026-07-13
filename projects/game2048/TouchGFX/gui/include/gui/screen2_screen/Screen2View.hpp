#ifndef SCREEN2VIEW_HPP
#define SCREEN2VIEW_HPP

#include <gui_generated/screen2_screen/Screen2ViewBase.hpp>
#include <gui/screen2_screen/Screen2Presenter.hpp>
#include <touchgfx/Callback.hpp>
#include <touchgfx/Unicode.hpp>
#include <touchgfx/widgets/AbstractButton.hpp>
#include <touchgfx/widgets/canvas/ScalableImage.hpp>

class Screen2View : public Screen2ViewBase
{
public:
    Screen2View();
    virtual ~Screen2View() {}

    virtual void setupScreen();
    virtual void tearDownScreen();

    void updateBoard(const uint32_t board[4][4]);
    void updateScore(uint32_t score);
    void updateHighScore(uint32_t score);

    void showGameOver();

protected:

private:
    static const uint16_t SCORE_BUFFER_SIZE = 12;
    Unicode::UnicodeChar scoreBuffer[SCORE_BUFFER_SIZE];
    Unicode::UnicodeChar highScoreBuffer[SCORE_BUFFER_SIZE];

    /* Bảng con trỏ 4×4 tới các ScalableImage sinh bởi Designer — điền
     * trong setupScreen. updateBoard chỉ cần loop qua bảng này. */
    touchgfx::ScalableImage* tiles[4][4];

    /* Callback chạm nút Return trên popup Game Over (Designer chưa wire). */
    touchgfx::Callback<Screen2View, const touchgfx::AbstractButton&> returnButtonCb;
    void returnButtonClicked(const touchgfx::AbstractButton& src);
};

#endif // SCREEN2VIEW_HPP
