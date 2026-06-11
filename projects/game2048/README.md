# Game 2048 trên STM32F429I-DISC1

Đồ án mô phỏng trò chơi 2048: điều khiển bằng **joystick analog (ADC)**, hiển thị
bằng cách **vẽ trực tiếp lên framebuffer LCD** (LTDC + SDRAM qua thư viện BSP, không TouchGFX),
sinh ô bằng **RNG phần cứng**. Đây là project STM32CubeIDE đã ghép code hoàn chỉnh.

## Kiến trúc phần mềm (tách lớp)

```
core/Inc/game2048.h   ┐  Lõi luật chơi: C THUẦN, không dính HAL.
core/Src/game2048.c   ┘  Test được trên Mac, tái dùng nguyên vẹn trên STM32.

core/Src/render.c     vẽ lưới + ô màu + điểm lên LCD bằng BSP_LCD_*
core/Src/joystick.c   đọc ADC 2 trục X/Y -> hướng LEFT/RIGHT/UP/DOWN (deadzone + edge-trigger)
core/Src/rng_hw.c     bọc RNG phần cứng cho con trỏ g2048_rng_fn
core/Src/app.c        vòng lặp game: đọc input -> g2048_move -> render
                      (được gọi từ main.c, USER CODE BEGIN 2)

core/Src/stm32f429i_discovery*.c, ili9341.c, fontXX.c   thư viện BSP LCD (từ CubeF4)
test/play_host.c      chơi thử trên terminal Mac (W/A/S/D) để kiểm chứng lõi
```

Quy tắc vàng: **mọi luật chơi nằm trong `game2048.c/.h`**, phần còn lại chỉ lo input + vẽ.
Sửa luật -> test trên Mac -> nạp xuống board.

## Test lõi game trên Mac

```bash
cd projects/game2048
clang -Wall -Wextra -I core/Inc core/Src/game2048.c test/play_host.c -o /tmp/play2048
/tmp/play2048      # W/A/S/D để chơi, Q để thoát
```

## Build & nạp trên STM32CubeIDE

1. Mở project `game2048` trong STM32CubeIDE.
2. Build (Ctrl+B). Cắm board F429I-DISC1, Run/Debug.
3. Joystick gạt 4 hướng để dồn số; nhấn nút **USER (PA0)** để chơi ván mới.

> Code game được gọi tại `Core/Src/main.c` trong khối `USER CODE BEGIN 2`
> (`app_run(&hadc1, &hrng);`). Vì đặt trước `osKernelStart()` nên FreeRTOS và
> USB Host (đồ thừa của template board) **không chạy** — vô hại. Khi Generate lại
> từ CubeMX, code trong USER CODE được giữ nguyên.

## Đấu dây phần cứng

| Chức năng     | Chân STM32       | Ghi chú                              |
|---------------|------------------|--------------------------------------|
| Joystick VRx  | PA5 (ADC1_IN5)   |                                      |
| Joystick VRy  | PC3 (ADC1_IN13)  |                                      |
| Joystick VCC  | **3V3**          | KHÔNG dùng 5V (ADC chỉ tới 3.3V)     |
| Joystick GND  | GND              |                                      |
| Joystick SW   | (bỏ trống)       | New Game dùng nút USER PA0           |
| Buzzer S/IO   | PE5 (TIM9_CH1)   | dành cho âm thanh (chưa code)        |
| Buzzer +/-    | 3V3 / GND        |                                      |

Kênh ADC khai báo trong `core/Inc/joystick.h` (`JOY_ADC_CH_X/Y`). Nếu đổi chân,
sửa 2 macro này cho khớp.

## Tinh chỉnh joystick (nếu cần)

- `JOY_THRESHOLD` / `JOY_DEADZONE` trong `joystick.h`: tăng nếu 1 cú gạt nhảy nhiều ô,
  hoặc nếu cần quá nhạy/quá ì.
- Edge-trigger: mỗi lần đẩy chỉ phát 1 nước; phải về vùng giữa rồi mới nhận lần đẩy tiếp.

## Tiến độ

- [x] Lõi game `game2048.[ch]` + test trên Mac
- [x] `render.c` — vẽ lưới 4x4 + màu theo giá trị + điểm + WIN/LOSE (BSP LCD)
- [x] `joystick.c` — ADC -> hướng (edge-trigger + deadzone)
- [x] `rng_hw.c` — RNG phần cứng cho lõi game
- [x] `app.c` — vòng lặp game + nút New Game (PA0)
- [x] Cấu hình CubeMX (ADC1 IN5/IN13, RNG, TIM9_CH1, clock 168 MHz)
- [x] Ghép BSP LCD + toàn bộ code vào project CubeIDE, nối `app_run` vào `main.c`
- [x] Kiểm chứng: lõi build & chạy trên Mac; mọi include phân giải với HAL thật
- [ ] **Build trong CubeIDE + nạp + test trên board thật** (bước tiếp theo)
- [ ] Hiệu chỉnh ngưỡng joystick theo module thực tế
- [ ] (Tuỳ chọn) âm thanh buzzer (PE5/TIM9_CH1) khi gộp ô / thắng / thua
- [ ] (Nâng cấp sau) chuyển sang TouchGFX + asset đẹp
```
