/*
 * joystick.h - Đọc joystick analog 2 trục qua ADC, trả về 1 hướng gạt.
 *
 * Nguyên tắc chống "trôi": edge-trigger + deadzone.
 *   - Cần phải về vùng giữa (deadzone) rồi đẩy mới tính 1 nước đi.
 *   - Giữ cần ở 1 phía sẽ KHÔNG sinh ra nhiều nước liên tiếp.
 *
 * Cấu hình trong CubeMX: bật ADC1, 2 kênh cho trục X và trục Y.
 * Sửa 2 macro kênh dưới đây cho khớp chân bạn chọn (xem .ioc / datasheet).
 */
#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "game2048.h"
#include "stm32f4xx_hal.h"

/* Kênh ADC cho 2 trục — khớp với chân CÒN TRỐNG trên F429I-DISC1 (xác nhận từ
 * CubeMX: IN5=PA5 và IN13=PC3 không bị LTDC/FMC/SPI5 chiếm). */
#ifndef JOY_ADC_CH_X
#define JOY_ADC_CH_X  ADC_CHANNEL_5    /* PA5 (trục X) */
#endif
#ifndef JOY_ADC_CH_Y
#define JOY_ADC_CH_Y  ADC_CHANNEL_13   /* PC3 (trục Y) */
#endif

/* Nút SW của joystick (ấn xuống stick) -> GPIO input, pull-up nội.
 * KY-023: SW thường nối GND khi ấn -> đọc 0 = đang ấn, 1 = thả.
 * Sửa 2 macro dưới nếu nối SW vào chân khác. PA6 là chân trống dễ tới,
 * sát PA5 đang dùng cho VRx. */
#ifndef JOY_SW_PORT
#define JOY_SW_PORT   GPIOA
#endif
#ifndef JOY_SW_PIN
#define JOY_SW_PIN    GPIO_PIN_6
#endif

/* Ngưỡng trên thang 12-bit (0..4095), tâm auto-calibrate tại boot. */
#define JOY_THRESHOLD  1200u           /* lệch quá mức này mới tính là "đẩy" */
#define JOY_DEADZONE   400u            /* trong khoảng tâm±400 coi như "thả" */

/* --- Hiệu chỉnh chiều theo cách lắp/đấu joystick (0 = giữ nguyên, 1 = bật) ---
 * Cách dò: cầm joystick cố định 1 chiều, build & test, đổi các số 0/1 dưới đây
 * cho tới khi cả 4 hướng đúng.
 *  - JOY_INVERT_Y: trục dọc bị ngược (đẩy lên thì đi xuống) -> để 1. Mặc định 1
 *    vì màn hình có trục Y hướng xuống, ngược với joystick "đẩy lên tăng VRy".
 *  - JOY_INVERT_X: trục ngang bị ngược (đẩy phải thì đi trái) -> để 1.
 *  - JOY_SWAP_XY : gạt ngang lại đi dọc và ngược lại (joystick lắp xoay 90°) -> để 1.
 */
#ifndef JOY_INVERT_X
#define JOY_INVERT_X  1
#endif
#ifndef JOY_INVERT_Y
#define JOY_INVERT_Y  1
#endif
#ifndef JOY_SWAP_XY
#define JOY_SWAP_XY   0
#endif

typedef enum {
    JOY_NONE = -1,
    JOY_DIR_LEFT  = G2048_LEFT,
    JOY_DIR_RIGHT = G2048_RIGHT,
    JOY_DIR_UP    = G2048_UP,
    JOY_DIR_DOWN  = G2048_DOWN
} joy_dir_t;

/* Lưu handle ADC do CubeMX sinh ra (vd. &hadc1). Gọi 1 lần lúc đầu.
 * Cũng cấu hình chân SW làm input pull-up. */
void joystick_init(ADC_HandleTypeDef *hadc);

/*
 * Quét joystick. Trả về JOY_NONE nếu không có sự kiện,
 * hoặc 1 hướng khi vừa đẩy qua ngưỡng (mỗi lần đẩy chỉ phát 1 lần).
 * Giá trị hướng trùng với g2048_dir_t -> ép kiểu trực tiếp được.
 */
joy_dir_t joystick_poll(void);

/*
 * Kiểm tra nút SW. Trả về true ĐÚNG 1 LẦN khi vừa được ấn xuống
 * (edge-trigger). Phải thả ra rồi ấn lại mới tính event tiếp theo.
 * Có debounce ~20ms để chống bouncing tiếp xúc cơ học.
 */
bool joystick_sw_pressed(void);

#endif /* JOYSTICK_H */
