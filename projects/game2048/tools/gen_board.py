"""Sinh preview 4x4 board theo layout mockup. Tile dùng lại từ gen_tiles."""
from PIL import Image, ImageDraw, ImageFont
from pathlib import Path
from gen_tiles import make_tile, TILES, SIZE, RADIUS

ROOT = Path(__file__).parent
OUT  = ROOT / "preview"

GAP        = 5
BOARD_PAD  = 8
BOARD_BG   = "#5C6A7A"     # slate blue-gray (sample từ mockup)
EMPTY_FILL = "#4B5366"     # slate đậm hơn (slot lõm xuống)
SLOT_TOP_SHADOW = "#2F3543"  # shadow đậm ở cạnh trên slot
SLOT_RAD   = 4
BOARD_RAD  = 8
TOP_SHADOW_H = 1            # độ dày shadow trên (px)

# Layout theo mockup
LAYOUT = [
    [2,  0,  0, 0],
    [0,  2,  0, 0],
    [4,  0, 16, 32],
    [0,  4,  0, 8],
]

W = 4 * SIZE + 3 * GAP + 2 * BOARD_PAD
H = W

board = Image.new("RGBA", (W, H), (0, 0, 0, 0))   # alpha 0 ở 4 góc
d = ImageDraw.Draw(board)
d.rounded_rectangle((0, 0, W - 1, H - 1), radius=BOARD_RAD, fill=BOARD_BG)

for i in range(4):
    for j in range(4):
        x = BOARD_PAD + j * (SIZE + GAP)
        y = BOARD_PAD + i * (SIZE + GAP)
        # Slot: shadow đậm ở cạnh trên (giả ánh sáng từ trên chiếu, bên trong tối)
        d.rounded_rectangle((x, y, x + SIZE - 1, y + SIZE - 1),
                            radius=SLOT_RAD, fill=SLOT_TOP_SHADOW)
        d.rounded_rectangle((x, y + TOP_SHADOW_H, x + SIZE - 1, y + SIZE - 1),
                            radius=SLOT_RAD, fill=EMPTY_FILL)
        v = LAYOUT[i][j]
        if v:
            bg, fg, fs = TILES[v]
            tile = make_tile(v, bg, fg, fs)
            board.paste(tile, (x, y), tile)

board.save(OUT / "_board_actual.png")

# Mô phỏng đúng cảnh trên LCD (kích cỡ thật, nền trắng)
scene = Image.new("RGBA", (240, 320), "#FFFFFF")
scene.paste(board, ((240 - W) // 2, 80), board)
scene.save(OUT / "_board_on_white.png")

# Bản zoom-pixel (chỉ để soi pixel)
big = board.resize((W * 4, H * 4), Image.NEAREST)
big.save(OUT / "_board_x4.png")

# Bản zoom-smooth (mô phỏng mắt nhìn)
smooth = board.resize((W * 3, H * 3), Image.LANCZOS)
smooth.save(OUT / "_board_smooth.png")

print(f"board {W}x{H} -> _board_actual.png  + _board_on_white.png  + _board_x4.png  + _board_smooth.png")
