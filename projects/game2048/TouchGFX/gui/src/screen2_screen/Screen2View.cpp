#include <gui/screen2_screen/Screen2View.hpp>
#include <touchgfx/Bitmap.hpp>
#include <images/BitmapDatabase.hpp>
#include <touchgfx/Unicode.hpp>

Screen2View::Screen2View()
{

}

void Screen2View::setupScreen()
{
    Screen2ViewBase::setupScreen();
}

void Screen2View::tearDownScreen()
{
    Screen2ViewBase::tearDownScreen();
}

static touchgfx::Bitmap getBitmapFromValue(uint32_t value)
{
    switch(value)
    {
    case 2:    return touchgfx::Bitmap(BITMAP_TITLE2_ID);
    case 4:    return touchgfx::Bitmap(BITMAP_TITLE4_ID);
    case 8:    return touchgfx::Bitmap(BITMAP_TITLE8_ID);
    case 16:   return touchgfx::Bitmap(BITMAP_TITLE16_ID);
    case 32:   return touchgfx::Bitmap(BITMAP_TITLE32_ID);
    case 64:   return touchgfx::Bitmap(BITMAP_TITLE64_ID);
    case 128:  return touchgfx::Bitmap(BITMAP_TITLE128_ID);
    case 256:  return touchgfx::Bitmap(BITMAP_TITLE256_ID);
    case 512:  return touchgfx::Bitmap(BITMAP_TITLE512_ID);
    case 1024: return touchgfx::Bitmap(BITMAP_TITLE1024_ID);
    case 2048: return touchgfx::Bitmap(BITMAP_TITLE2048_ID);
    case 4096: return touchgfx::Bitmap(BITMAP_TITLE4096_ID);
    case 8192: return touchgfx::Bitmap(BITMAP_TITLE8192_ID);

    default:
        return touchgfx::Bitmap();
    }
}

static void setTile(touchgfx::ScalableImage& img,
                    uint32_t value)
{
    if(value == 0)
    {
        img.setVisible(false);
        return;
    }

    img.setVisible(true);
    img.setBitmap(getBitmapFromValue(value));
}

void Screen2View::updateBoard(const uint32_t board[4][4])
{
    setTile(scalableImage00, board[0][0]);
    setTile(scalableImage01, board[0][1]);
    setTile(scalableImage02, board[0][2]);
    setTile(scalableImage03, board[0][3]);

    setTile(scalableImage10, board[1][0]);
    setTile(scalableImage11, board[1][1]);
    setTile(scalableImage12, board[1][2]);
    setTile(scalableImage13, board[1][3]);

    setTile(scalableImage20, board[2][0]);
    setTile(scalableImage21, board[2][1]);
    setTile(scalableImage22, board[2][2]);
    setTile(scalableImage23, board[2][3]);

    setTile(scalableImage30, board[3][0]);
    setTile(scalableImage31, board[3][1]);
    setTile(scalableImage32, board[3][2]);
    setTile(scalableImage33, board[3][3]);

    invalidate();
}

void Screen2View::updateScore(uint32_t score)
{
    Unicode::snprintf(scoreBuffer,
                      SCORE_BUFFER_SIZE,
                      "%lu",
                      score);

    textArea2.invalidate();
}
