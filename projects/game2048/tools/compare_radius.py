"""So sánh 3 board cùng tile size, khác RADIUS, ở scale x3 LANCZOS để dễ nhìn."""
import gen_tiles
from PIL import Image, ImageDraw
from pathlib import Path

OUT = Path(__file__).parent / "preview"

GAP, PAD = 5, 8

def board(radius):
    gen_tiles.RADIUS = radius
    SIZE = gen_tiles.SIZE
    W = 4 * SIZE + 3 * GAP + 2 * PAD
    bd = Image.new("RGBA", (W, W), (0, 0, 0, 0))
    d = ImageDraw.Draw(bd)
    d.rounded_rectangle((0, 0, W-1, W-1), radius=10, fill="#5A5F66")
    LAYOUT = [[2,0,0,0],[0,2,0,0],[4,0,16,32],[0,4,0,8]]
    for i in range(4):
        for j in range(4):
            x = PAD + j*(SIZE+GAP); y = PAD + i*(SIZE+GAP)
            d.rounded_rectangle((x, y, x+SIZE-1, y+SIZE-1), radius=radius, fill="#2E3338")
            v = LAYOUT[i][j]
            if v:
                bg, fg, fs = gen_tiles.TILES[v]
                t = gen_tiles.make_tile(v, bg, fg, fs)
                bd.paste(t, (x, y), t)
    return bd

bds = [(r, board(r)) for r in (4, 6, 8)]
W = bds[0][1].width
gap = 20
canvas = Image.new("RGBA", (W*3 + gap*4, W + 40), "#FFFFFF")
d = ImageDraw.Draw(canvas)
for i, (r, b) in enumerate(bds):
    x = gap + i*(W+gap)
    canvas.paste(b, (x, 30), b)
    d.text((x + W//2 - 20, 8), f"radius={r}", fill="#000000")

canvas = canvas.resize((canvas.width*3, canvas.height*3), Image.LANCZOS)
canvas.save(OUT / "_radius_compare.png")
print("wrote", OUT/"_radius_compare.png")
