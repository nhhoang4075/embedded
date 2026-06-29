"""Sinh tile PNG cho 2048 theo style mockup.
Ghi vào tools/preview/ trước để xem thử. Khi ưng -> đổi OUT thành assets/images.
"""
from PIL import Image, ImageDraw, ImageFont
from pathlib import Path

SIZE   = 40          # board 4*40 + 3*5 + 2*8 = 191 -> padding ~24px mỗi bên
RADIUS = 4
SHADOW = 2           # px shadow dưới + phải của tile (hiệu ứng nổi 3D)
ROOT   = Path(__file__).parent
OUT    = ROOT / "preview"

TILES = {
    # Đi vòng quanh hue wheel: mỗi giá trị 1 vùng màu khác hẳn -> không nhầm
    2:    ("#F8B888", "#FFFFFF", 22),   # peach
    4:    ("#F7D344", "#FFFFFF", 22),   # sunflower
    8:    ("#FF8C2B", "#FFFFFF", 22),   # vivid orange
    16:   ("#22D3EE", "#FFFFFF", 19),   # bright cyan
    32:   ("#E83E8C", "#FFFFFF", 19),   # hot pink
    64:   ("#8B5CF6", "#FFFFFF", 19),   # violet
    128:  ("#4ADE80", "#FFFFFF", 15),   # bright green
    256:  ("#14B8A6", "#FFFFFF", 15),   # teal
    512:  ("#A3E635", "#FFFFFF", 15),   # lime
    1024: ("#4338CA", "#FFFFFF", 13),   # indigo đậm
    2048: ("#EAB308", "#FFFFFF", 13),   # gold (giải thưởng)
    4096: ("#DC2626", "#FFFFFF", 13),   # crimson
    8192: ("#BE185D", "#FFFFFF", 13),   # deep raspberry (legendary)
}

FONT_PATH = "/System/Library/Fonts/Supplemental/Arial Bold.ttf"  # stroke mảnh hơn Verdana Bold
SUPERSAMPLE = 4   # render lớn 4x rồi LANCZOS xuống -> sắc nét hơn render trực tiếp

def _darken(hex_color, k=0.65):
    """Trộn màu với đen theo hệ số k (0..1, càng thấp càng tối)."""
    r = int(hex_color[1:3], 16); g = int(hex_color[3:5], 16); b = int(hex_color[5:7], 16)
    return (int(r*k), int(g*k), int(b*k), 255)

def _lighten(hex_color, k=0.18):
    r = int(hex_color[1:3], 16); g = int(hex_color[3:5], 16); b = int(hex_color[5:7], 16)
    r = int(r + (255 - r) * k); g = int(g + (255 - g) * k); b = int(b + (255 - b) * k)
    return (r, g, b, 255)

def make_tile(value, bg, fg, fsize):
    # Vẽ rounded rect ở size gốc (PIL AA nội bộ -> không LANCZOS overshoot).
    # Text render riêng ở supersample size rồi paste -> chữ vẫn sắc.
    tile = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
    ImageDraw.Draw(tile).rounded_rectangle(
        (0, 0, SIZE-1, SIZE-1), radius=RADIUS, fill=bg)

    # Render text ở SS canvas trong suốt rồi LANCZOS xuống.
    S = SIZE * SUPERSAMPLE
    text_layer = Image.new("RGBA", (S, S), (0, 0, 0, 0))
    d_t = ImageDraw.Draw(text_layer)
    font = ImageFont.truetype(FONT_PATH, fsize * SUPERSAMPLE)
    text = str(value)
    bb = d_t.textbbox((0, 0), text, font=font)
    w, h = bb[2] - bb[0], bb[3] - bb[1]
    tx = (S - w) // 2 - bb[0]
    ty = (S - h) // 2 - bb[1] - SUPERSAMPLE
    d_t.text((tx, ty), text, font=font, fill=fg)
    text_small = text_layer.resize((SIZE, SIZE), Image.LANCZOS)

    tile.alpha_composite(text_small)
    return tile

OUT.mkdir(parents=True, exist_ok=True)
imgs = []
for v, (bg, fg, fs) in TILES.items():
    im = make_tile(v, bg, fg, fs)
    im.save(OUT / f"tile{v}.png")
    imgs.append((v, im))

# Montage 4x4 trên nền xám đậm giống mockup, có 3 ô trống
GAP = 6
COLS = 5
ROWS = 3
W = COLS * (SIZE + GAP) + GAP
H = ROWS * (SIZE + GAP) + GAP
board = Image.new("RGBA", (W, H), "#3A3F44")
d = ImageDraw.Draw(board)
for i in range(ROWS):
    for j in range(COLS):
        x = GAP + j * (SIZE + GAP)
        y = GAP + i * (SIZE + GAP)
        d.rounded_rectangle((x, y, x + SIZE - 1, y + SIZE - 1),
                            radius=RADIUS, fill="#4A4F55")
for idx, (_, im) in enumerate(imgs[:ROWS * COLS]):
    i, j = divmod(idx, COLS)
    x = GAP + j * (SIZE + GAP)
    y = GAP + i * (SIZE + GAP)
    board.paste(im, (x, y), im)

# Scale x4 để dễ xem
board = board.resize((W * 4, H * 4), Image.NEAREST)
board.save(OUT / "_preview.png")
print(f"wrote {len(imgs)} tiles + montage to {OUT}")
