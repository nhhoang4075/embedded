"""Sinh UI asset: card SCORE/BEST + button PLAY NOW + icon RESTART.
Màu lấy từ mockup. Size cho LCD 240x320.
"""
from PIL import Image, ImageDraw, ImageFont
from pathlib import Path

OUT = Path(__file__).parent / "preview"
OUT.mkdir(exist_ok=True)

FONT_BOLD   = "/System/Library/Fonts/Supplemental/Arial Bold.ttf"
FONT_REG    = "/System/Library/Fonts/Supplemental/Arial.ttf"
SUPERSAMPLE = 4

# ---------- Helpers ----------
def make_card(w, h, radius, bg, path):
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    ImageDraw.Draw(img).rounded_rectangle((0, 0, w-1, h-1), radius=radius, fill=bg)
    img.save(path)
    return img

def make_button(w, h, radius, bg, shadow, path):
    """Button có shadow dày 2px ở cạnh dưới."""
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    d.rounded_rectangle((0, 2, w-1, h-1), radius=radius, fill=shadow)
    d.rounded_rectangle((0, 0, w-1, h-3), radius=radius, fill=bg)
    img.save(path)
    return img

def draw_text_centered(img, text, font_path, size, color, y_offset=0):
    d = ImageDraw.Draw(img)
    f = ImageFont.truetype(font_path, size)
    bb = d.textbbox((0, 0), text, font=f)
    tw, th = bb[2] - bb[0], bb[3] - bb[1]
    x = (img.width  - tw) // 2 - bb[0]
    y = (img.height - th) // 2 - bb[1] + y_offset
    d.text((x, y), text, font=f, fill=color)

def make_restart_icon(size, color, path):
    """Vẽ icon refresh đơn giản: vòng tròn hở + mũi tên."""
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    pad = 2
    # Vòng tròn hở (arc từ 30° đến 320°)
    d.arc((pad, pad, size-pad-1, size-pad-1), start=30, end=320, fill=color, width=3)
    # Mũi tên ở đầu (cuối arc 320°)
    import math
    cx, cy = size/2, size/2
    r = (size - 2*pad) / 2
    ang = math.radians(320)
    tx = cx + r * math.cos(ang)
    ty = cy + r * math.sin(ang)
    # Tam giác mũi tên
    d.polygon([(tx-3, ty-1), (tx+3, ty-2), (tx, ty+4)], fill=color)
    img.save(path)
    return img

# ---------- Asset preview ----------
# 1. Card SCORE (xanh nhạt) - card trắng, không có số bên trong
card_score = make_card(70, 22, 4, "#C6E1FF", OUT / "card_score.png")

# 2. Card BEST (xanh đậm) - không có số bên trong
card_best = make_card(70, 22, 4, "#1B6ED4", OUT / "card_best.png")

# 3. Button PLAY NOW - bo ít hơn (radius 8 -> 4)
btn_rel = make_button(200, 32, 4, "#0F7CFD", "#0557C3", OUT / "btn_play_released.png")
draw_text_centered(btn_rel, "PLAY NOW", FONT_BOLD, 14, "#FFFFFF", y_offset=-1)
btn_rel.save(OUT / "btn_play_released.png")

# Pressed: nền đậm hơn, không shadow
btn_pre = Image.new("RGBA", (200, 32), (0,0,0,0))
ImageDraw.Draw(btn_pre).rounded_rectangle((0,2,199,31), radius=4, fill="#0557C3")
draw_text_centered(btn_pre, "PLAY NOW", FONT_BOLD, 14, "#FFFFFF", y_offset=0)
btn_pre.save(OUT / "btn_play_pressed.png")

# 4. Restart icon
make_restart_icon(24, "#1B6ED4", OUT / "icon_restart.png")

# 5. Title "2048" - render PNG riêng, dùng supersample để chữ sắc
def make_title_png(text, font_path, fsize, color, path, padding=4):
    """Render text PNG, transparent bg, size vừa khít text + padding."""
    # Ước lượng width/height bằng cách render thử
    tmp_img = Image.new("RGBA", (10, 10))
    tmp_d   = ImageDraw.Draw(tmp_img)
    f       = ImageFont.truetype(font_path, fsize)
    bb      = tmp_d.textbbox((0, 0), text, font=f)
    w       = int(bb[2] - bb[0] + padding*2)
    h       = int(bb[3] - bb[1] + padding*2)
    # Supersample render
    S = SUPERSAMPLE
    big = Image.new("RGBA", (w*S, h*S), (0, 0, 0, 0))
    d_b = ImageDraw.Draw(big)
    f_b = ImageFont.truetype(font_path, fsize * S)
    bb_b = d_b.textbbox((0, 0), text, font=f_b)
    d_b.text((padding*S - bb_b[0], padding*S - bb_b[1]), text, font=f_b, fill=color)
    small = big.resize((w, h), Image.LANCZOS)
    small.save(path)
    return small

title = make_title_png("2048", FONT_BOLD, 46, "#1B6ED4", OUT / "title_2048.png")
print(f" title 2048 -> {title.size}")

# ---------- Montage để xem cùng nhau ----------
LCD_W, LCD_H = 240, 320
scene = Image.new("RGBA", (LCD_W, LCD_H), "#F0F5FB")  # nền giống mockup

# Title "2048" từ PNG đã render sẵn
title_x = (LCD_W - title.width) // 2
scene.paste(title, (title_x, 2), title)

# Labels SCORE / BEST
lbl_img = Image.new("RGBA", (LCD_W, 14), (0,0,0,0))
d = ImageDraw.Draw(lbl_img)
f = ImageFont.truetype(FONT_BOLD, 10)
d.text((24,  2), "SCORE", font=f, fill="#8896A8")
d.text((110, 2), "BEST",  font=f, fill="#8896A8")
scene.paste(lbl_img, (0, 56), lbl_img)

# Cards (chiều cao 22)
scene.paste(card_score, (16,  72), card_score)
scene.paste(card_best,  (94,  72), card_best)

# Restart icon
restart = Image.open(OUT / "icon_restart.png")
scene.paste(restart, (200, 72), restart)

# Board (191x191)
board = Image.open(OUT / "_board_actual.png")
scene.paste(board, ((LCD_W - board.width)//2, 96), board)

# PLAY NOW button
btn = Image.open(OUT / "btn_play_released.png")
scene.paste(btn, ((LCD_W - btn.width)//2, 290), btn)

scene.save(OUT / "_scene.png")
scene_big = scene.resize((LCD_W*3, LCD_H*3), Image.LANCZOS)
scene_big.save(OUT / "_scene_smooth.png")

print("OK:")
print(" cards/button/icon ->", OUT / "card_*.png", OUT / "btn_play_*.png", OUT / "icon_restart.png")
print(" full scene mockup ->", OUT / "_scene.png  (+_scene_smooth.png)")
