# Ghép firmware 2048 vào project STM32CubeIDE

Các file ở thư mục này (`firmware/`) + lõi ở `../core/` là code thuần, **không** do
CubeMX sinh. Dưới đây là cách nhét chúng vào một project CubeIDE cho F429I-DISCO.

## 1. Tạo project & bật ngoại vi (CubeMX)

1. **File > New > STM32 Project > Board Selector > STM32F429I-DISCO** → Yes (khởi tạo
   ngoại vi mặc định: LTDC, FMC/SDRAM, SPI5 cho LCD đã sẵn). **Không** bật TouchGFX.
2. **ADC1**: bật, tick 2 kênh cho trục X, Y của joystick.
   - Chọn 2 chân ADC còn trống, **tránh** các chân LCD/SDRAM và chân gyro (PA1, PA2).
   - Ghi lại số kênh (vd. PA5 = `ADC_CHANNEL_5`, PA6 = `ADC_CHANNEL_6`).
3. **RNG**: Analog > RNG > Activated.
4. Nút USER (PA0) board đã cấu hình là input sẵn — dùng để "chơi lại".
5. **Project Manager > Code Generator**: tick *Copy only the necessary library files*
   và *Generate peripheral init as a pair of .c/.h*. Generate Code.

## 2. Thêm thư viện BSP LCD (vẽ trực tiếp)

Từ gói CubeF4 (thường ở `~/STM32Cube/Repository/STM32Cube_FW_F4_Vxxx/`):

- Copy `Drivers/BSP/STM32F429I-Discovery/stm32f429i_discovery_lcd.{c,h}`
  và `stm32f429i_discovery.{c,h}` (+ `_sdram` nếu cần) vào `Drivers/BSP/...` của project.
- Copy `Drivers/BSP/Components/ili9341/` và `Utilities/Fonts/` (font.c, fonts.h).
- Trong project Properties > C/C++ > Paths and Symbols, thêm include path tới các thư
  mục Fonts và BSP vừa copy.

> Mẹo: cách nhanh nhất là dùng đúng cấu trúc mà Lab 3 (`SimpleRacing`) đã có sẵn LCD BSP.

## 3. Thêm code game vào project

- Copy `core/game2048.{c,h}` và `firmware/*.{c,h}` vào `Core/Src` và `Core/Inc`
  (hoặc tạo group `game/` rồi thêm include path).
- Mở `firmware/joystick.h`, sửa `JOY_ADC_CH_X` / `JOY_ADC_CH_Y` cho khớp kênh ở bước 1.

## 4. Gọi game từ main.c

Trong `Core/Src/main.c`, thêm vào vùng USER CODE:

```c
/* USER CODE BEGIN Includes */
#include "app.h"
/* USER CODE END Includes */
```

Sau tất cả `MX_*_Init()` (cuối hàm `main`, trước `while(1)`):

```c
  /* USER CODE BEGIN 2 */
  app_run(&hadc1, &hrng);   /* không trả về */
  /* USER CODE END 2 */
```

> `hadc1`, `hrng` là tên biến CubeMX sinh ra. Nếu khác (vd. `hadc2`) thì sửa cho khớp.

## 5. Build & nạp

- Build (Ctrl+B). Cắm board, Run/Debug.
- Joystick gạt 4 hướng để dồn số; nhấn nút USER để chơi ván mới.

## Bố cục chân (gợi ý, chốt lại theo .ioc thực tế)

| Chức năng        | Chân gợi ý      | Ghi chú                        |
|------------------|-----------------|--------------------------------|
| Joystick VRx     | PA5 (ADC1_IN5)  | dòng trắng (trống) trong CubeMX |
| Joystick VRy     | PC3 (ADC1_IN13) | dòng trắng (trống) trong CubeMX |
| Joystick GND/VCC | GND / 3V3       |                                |
| Nút "chơi lại"   | PA0 (USER)      | có sẵn trên board              |

## Khắc phục sự cố nhanh

- **Màn đen**: thiếu BSP LCD init hoặc SDRAM chưa bật → kiểm tra FMC trong .ioc.
- **Số ô không đổi khi gạt**: sai kênh ADC, hoặc joystick chưa nối GND chung →
  đo thử giá trị `read_channel()` (đặt breakpoint), kỳ vọng ~2048 khi thả.
- **Gạt 1 cái nhảy nhiều ô**: tăng `JOY_DEADZONE`/`JOY_THRESHOLD` trong `joystick.h`.
- **Sinh ô không ngẫu nhiên / đứng hình**: RNG chưa bật trong CubeMX.
```
