#include <gui/screen2_screen/Screen2View.hpp>
#include <gui/common/FrontendApplication.hpp>
#include <touchgfx/Bitmap.hpp>
#include <images/BitmapDatabase.hpp>
#include <touchgfx/Unicode.hpp>

/* Ánh xạ giá trị ô → bitmap id. Thêm mức mới chỉ cần một dòng — không
 * đụng logic. */
namespace {

struct TileBitmap {
    uint32_t value;
    uint16_t bitmapId;
};

constexpr TileBitmap kTileBitmaps[] = {
    {   2,  BITMAP_TITLE2_ID    },
    {   4,  BITMAP_TITLE4_ID    },
    {   8,  BITMAP_TITLE8_ID    },
    {  16,  BITMAP_TITLE16_ID   },
    {  32,  BITMAP_TITLE32_ID   },
    {  64,  BITMAP_TITLE64_ID   },
    { 128,  BITMAP_TITLE128_ID  },
    { 256,  BITMAP_TITLE256_ID  },
    { 512,  BITMAP_TITLE512_ID  },
    {1024,  BITMAP_TITLE1024_ID },
    {2048,  BITMAP_TITLE2048_ID },
    {4096,  BITMAP_TITLE4096_ID },
    {8192,  BITMAP_TITLE8192_ID },
};

touchgfx::Bitmap getBitmapFromValue(uint32_t value)
{
    for (const auto& t : kTileBitmaps) {
        if (t.value == value) return touchgfx::Bitmap(t.bitmapId);
    }
    return touchgfx::Bitmap();
}

void setTile(touchgfx::ScalableImage& img, uint32_t value)
{
    if (value == 0) {
        img.setVisible(false);
        return;
    }
    img.setVisible(true);
    img.setBitmap(getBitmapFromValue(value));
}

} // namespace

Screen2View::Screen2View() { }

void Screen2View::setupScreen()
{
    Screen2ViewBase::setupScreen();

    /* Nạp mảng 16 con trỏ ScalableImage một lần khi setup — sau đó
     * updateBoard chỉ cần loop. Muốn đổi lưới 5×5 chỉ cần sinh các
     * scalableImageRC bằng Designer rồi thêm ở đây. */
    tiles[0][0] = &scalableImage00; tiles[0][1] = &scalableImage01;
    tiles[0][2] = &scalableImage02; tiles[0][3] = &scalableImage03;
    tiles[1][0] = &scalableImage10; tiles[1][1] = &scalableImage11;
    tiles[1][2] = &scalableImage12; tiles[1][3] = &scalableImage13;
    tiles[2][0] = &scalableImage20; tiles[2][1] = &scalableImage21;
    tiles[2][2] = &scalableImage22; tiles[2][3] = &scalableImage23;
    tiles[3][0] = &scalableImage30; tiles[3][1] = &scalableImage31;
    tiles[3][2] = &scalableImage32; tiles[3][3] = &scalableImage33;

    textArea2.setWildcard(scoreBuffer);
    textArea1.setWildcard(highScoreBuffer);
    Unicode::snprintf(scoreBuffer,     SCORE_BUFFER_SIZE, "0");
    Unicode::snprintf(highScoreBuffer, SCORE_BUFFER_SIZE, "0");

    textArea2.resizeToCurrentText();
    textArea2.invalidate();
    textArea1.resizeToCurrentText();
    textArea1.invalidate();

    returnButtonCb = touchgfx::Callback<Screen2View, const touchgfx::AbstractButton&>(
        this, &Screen2View::returnButtonClicked);
    returnButton.setAction(returnButtonCb);
}

void Screen2View::returnButtonClicked(const touchgfx::AbstractButton& /*src*/)
{
    static_cast<FrontendApplication*>(touchgfx::Application::getInstance())
        ->gotoScreen1ScreenNoTransition();
}

void Screen2View::tearDownScreen()
{
    Screen2ViewBase::tearDownScreen();
}

void Screen2View::updateBoard(const uint32_t board[4][4])
{
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            setTile(*tiles[r][c], board[r][c]);
        }
    }
    invalidate();
}

void Screen2View::updateScore(uint32_t score)
{
    Unicode::snprintf(scoreBuffer, SCORE_BUFFER_SIZE, "%u", score);
    textArea2.resizeToCurrentText();
    textArea2.invalidate();
}

void Screen2View::updateHighScore(uint32_t score)
{
    Unicode::snprintf(highScoreBuffer, SCORE_BUFFER_SIZE, "%u", score);
    textArea1.resizeToCurrentText();
    textArea1.invalidate();
}

void Screen2View::showGameOver()
{
    containerGameOver.setVisible(true);
    containerGameOver.invalidate();
}
