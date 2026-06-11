/*
 * render.c - Cài đặt vẽ giao diện 2048 bằng BSP LCD (ARGB8888).
 *
 * Màn F429I-DISCO: 240 x 320 (dọc). Bố cục:
 *   y 0..58   : tiêu đề "2048" + điểm
 *   y 64..286 : lưới 4x4 (mỗi ô 48px, khe 6px, lề 8px)
 *   y 290..   : dòng thông báo trạng thái
 */
#include "render.h"
#include "stm32f429i_discovery_lcd.h"

/* ----- Hình học lưới ----- */
#define BOARD_X       8u
#define BOARD_Y       64u
#define GAP           6u
#define TILE          48u
#define CELL_STEP     (TILE + GAP)     /* khoảng cách giữa 2 ô */

/* Toạ độ góc trên-trái của ô (row,col) trên màn hình */
#define CELL_PX(col)  (BOARD_X + GAP + (col) * CELL_STEP)
#define CELL_PY(row)  (BOARD_Y + GAP + (row) * CELL_STEP)

/* ----- Màu (ARGB8888) theo phong cách 2048 gốc ----- */
#define COL_BG        0xFFFAF8EFu   /* nền toàn màn */
#define COL_BOARD     0xFFBBADA0u   /* nền khung lưới */
#define COL_EMPTY     0xFFCDC1B4u   /* ô trống */
#define COL_TEXT_DARK 0xFF776E65u   /* số trên ô sáng (2,4) */
#define COL_TEXT_LIGHT 0xFFF9F6F2u  /* số trên ô đậm (>=8) */

/* uint -> chuỗi thập phân. Trả về độ dài. buf phải đủ chỗ (>=11). */
static int u32_to_str(uint32_t v, char *buf)
{
    char tmp[11];
    int n = 0;
    if (v == 0) { buf[0] = '0'; buf[1] = '\0'; return 1; }
    while (v > 0) { tmp[n++] = (char)('0' + (v % 10u)); v /= 10u; }
    for (int i = 0; i < n; i++) buf[i] = tmp[n - 1 - i];
    buf[n] = '\0';
    return n;
}

/* Chọn màu nền ô theo giá trị. */
static uint32_t tile_bg(uint32_t v)
{
    switch (v) {
    case 2:    return 0xFFEEE4DAu;
    case 4:    return 0xFFEDE0C8u;
    case 8:    return 0xFFF2B179u;
    case 16:   return 0xFFF59563u;
    case 32:   return 0xFFF67C5Fu;
    case 64:   return 0xFFF65E3Bu;
    case 128:  return 0xFFEDCF72u;
    case 256:  return 0xFFEDCC61u;
    case 512:  return 0xFFEDC850u;
    case 1024: return 0xFFEDC53Fu;
    case 2048: return 0xFFEDC22Eu;
    default:   return 0xFF3C3A32u;   /* >2048: ô tối */
    }
}

void render_init(void)
{
    BSP_LCD_Init();
    BSP_LCD_LayerDefaultInit(LCD_BACKGROUND_LAYER, LCD_FRAME_BUFFER);
    BSP_LCD_SelectLayer(LCD_BACKGROUND_LAYER);
    BSP_LCD_DisplayOn();
    BSP_LCD_Clear(COL_BG);
}

/* Vẽ một ô (số) tại vị trí lưới (row,col). value==0 -> ô trống. */
static void draw_tile(int row, int col, uint32_t value)
{
    uint16_t x = CELL_PX(col);
    uint16_t y = CELL_PY(row);

    /* Nền ô */
    BSP_LCD_SetTextColor(value == 0 ? COL_EMPTY : tile_bg(value));
    BSP_LCD_FillRect(x, y, TILE, TILE);

    if (value == 0)
        return;

    /* Số căn giữa ô */
    char buf[11];
    int len = u32_to_str(value, buf);

    /* Chọn font theo số chữ số để vừa ô 48px. */
    sFONT *font = &Font24;          /* rộng 17px */
    if (len >= 3) font = &Font16;   /* rộng 11px */
    if (len >= 5) font = &Font12;   /* rộng 7px  */
    BSP_LCD_SetFont(font);

    uint16_t tw = (uint16_t)(len * font->Width);
    uint16_t th = font->Height;
    uint16_t tx = (uint16_t)(x + (TILE - tw) / 2);
    uint16_t ty = (uint16_t)(y + (TILE - th) / 2);

    BSP_LCD_SetBackColor(tile_bg(value));
    BSP_LCD_SetTextColor(value <= 4 ? COL_TEXT_DARK : COL_TEXT_LIGHT);
    BSP_LCD_DisplayStringAt(tx, ty, (uint8_t *)buf, LEFT_MODE);
}

void render_board(const g2048_game_t *g)
{
    /* Khung nền lưới */
    BSP_LCD_SetTextColor(COL_BOARD);
    BSP_LCD_FillRect(BOARD_X, BOARD_Y,
                     GAP + G2048_SIZE * CELL_STEP,
                     GAP + G2048_SIZE * CELL_STEP);

    /* Tiêu đề + điểm */
    BSP_LCD_SetBackColor(COL_BG);
    BSP_LCD_SetTextColor(COL_TEXT_DARK);
    BSP_LCD_SetFont(&Font24);
    BSP_LCD_DisplayStringAt(BOARD_X, 6, (uint8_t *)"2048", LEFT_MODE);

    char line[24];
    char num[11];
    u32_to_str(g->score, num);
    /* "Score: <n>" */
    const char *p = "Score: ";
    int k = 0;
    while (*p) line[k++] = *p++;
    for (int i = 0; num[i]; i++) line[k++] = num[i];
    line[k] = '\0';
    BSP_LCD_SetFont(&Font16);
    /* xoá dải điểm cũ trước khi vẽ lại để tránh số thừa */
    BSP_LCD_SetTextColor(COL_BG);
    BSP_LCD_FillRect(BOARD_X, 38, BSP_LCD_GetXSize() - 2 * BOARD_X, 18);
    BSP_LCD_SetTextColor(COL_TEXT_DARK);
    BSP_LCD_DisplayStringAt(BOARD_X, 38, (uint8_t *)line, LEFT_MODE);

    /* Các ô */
    for (int r = 0; r < G2048_SIZE; r++)
        for (int c = 0; c < G2048_SIZE; c++)
            draw_tile(r, c, g->grid[r][c]);

    /* Dòng thông báo trạng thái */
    uint16_t msg_y = BOARD_Y + GAP + G2048_SIZE * CELL_STEP + 6;
    BSP_LCD_SetTextColor(COL_BG);
    BSP_LCD_FillRect(0, msg_y, BSP_LCD_GetXSize(), 20);
    BSP_LCD_SetBackColor(COL_BG);
    BSP_LCD_SetFont(&Font16);
    if (g->state == G2048_WON) {
        BSP_LCD_SetTextColor(0xFF2E8B57u);
        BSP_LCD_DisplayStringAt(BOARD_X, msg_y, (uint8_t *)"YOU WIN!", LEFT_MODE);
    } else if (g->state == G2048_LOST) {
        BSP_LCD_SetTextColor(0xFFB22222u);
        BSP_LCD_DisplayStringAt(BOARD_X, msg_y, (uint8_t *)"GAME OVER - btn=new", LEFT_MODE);
    }
}
